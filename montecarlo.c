#include <math.h>
#include <string.h>

#include "lderiv.h"
#include "slist.h"
#include "mem.h"
#include "assert.h"
#include "safeint.h"
#include "table.h"
#include "str.h"
#include "atom.h"
#include "portable.h"
#include "safeint.h"

int zbrac(double (*func) (double, void*), double* x1, double* x2, void* e) {
    int j;
    double f1, f2;

    assert(*x1 != *x2);

    f1 = func(*x1, e);
    f2 = func(*x2, e);

    for(j = 1; j <= 50; j++) {
        if(f1 * f2 < 0.0) return 1;
        if(fabs(f1) < fabs(f2))
            f1 = func(*x1 += 1.6 * (*x1 - *x2), e);
        else
            f2 = func(*x2 += 1.6 * (*x2 - *x1), e);
    }
    return 0;
}

double rtbis(double (*func) (double, void*), double x1, double x2, double xacc, void* e) {
    int j;
    double dx, f, fmid, xmid, rtb;

    f = func(x1, e);
    fmid = func(x2, e);
    assert(f * fmid < 0.0);

    rtb = f < 0.0 ? (dx = x2 - x1, x1) : (dx = x1 - x2, x2);

    for(j = 1; j <= 40; j++) {
        fmid = func(xmid = rtb + (dx *= 0.5), e);
        if(fmid <= 0.0) rtb = xmid;
        if(fabs(dx) < xacc || fmid == 0.0) return rtb;
    }
    assert(0);
    return 0;
}

double interval_bisection(double y_target,  // Target y value
                          double m,         // Left interval value
                          double n,         // Right interval value
                          double epsilon,   // Tolerance
                          double (*g) (double, void*),
                          void* env) {            // Function pointer

    double x = 0.5 * (m + n);
    double y = g(x, env);

    do {
        if (y < y_target) {
          m = x;
        }

        if (y > y_target) {
          n = x;
        }

        x = 0.5 * (m + n);
        y = g(x, env);
    } while (fabs(y-y_target) > epsilon);

    return x;
}

static struct envStruct {
    Contract* Contract;
    double Price;
    double Cost;
    double RiskFree;
    double Time;
    double Yield;
    time_t EvalTime;
    Table_T Result;
};

static double bs_wrapper(double vol, void* cl) {
    struct envStruct* e = (struct envStruct*) cl;
    Contract* c = e->Contract;
    char type = c->Kind == Contract_CallOption ? 'c' : 'p';

    return e->Cost - black_scholesG(type, e->Price, c->Strike, e->Time, e->RiskFree, e->Yield - e->RiskFree, vol);
    
}

void findVol(void** x, void* cl) {

    double* vol;
    Position* p = (Position*) *x;
    Contract* c = p->Contract;
    struct envStruct* e = (struct envStruct*) cl;
    int foundBrackets;
    double x1 = 0.01, x2 = 2.00;

    e->Contract = c;
    e->Time = diff_in_years(c->Expiry, e->EvalTime);
    e->Cost = p->CostPerContract;

    NEW(vol);
    //*vol = interval_bisection(0, 0.01, 2.0, p->CostPerContract * 0.1, bs_wrapper, e);
    foundBrackets = zbrac(bs_wrapper, &x1, &x2, e);
    *vol = rtbis(bs_wrapper, x1, x2, 0.0001, e);

    Table_put(e->Result, c, vol);
}

void* CalcPerStrikeVol(SList_T port, double price, time_t evalTime, double riskFree, double yield) {

    Table_T table = Table_new(10, NULL, NULL);
    struct envStruct e = { NULL, price, 0, riskFree, 0, yield, evalTime, table }; // Position, cost and time to expiry are contract specific
    
    SList_map(port, findVol, &e);

    return table;
}

void freeVol(const void* key, void** value, void* cl) {
    FREE(*value);
}

double* MC_RunSingleT(SList_T port,
                      Process_Kind procKind,
                      void* env,
                      Contract_VolCalculation volCalc,
                      void* volEnv,
                      double initialPrice,
                      time_t evalTime,
                      double riskFree,
                      double yield,
                      double vol,
                      unsigned iters,
                      unsigned days) {
    unsigned i, d;
    double price;
    double* payoffs;
    void* volEnv1;
    int volsAssigned = 0;

    assert(port);
    safe_mul_uu(iters, days);
    payoffs = CALLOC(iters * days, sizeof(*payoffs)); 

    if(volCalc == Vol_ConstantByStrike && !volEnv) {
       volsAssigned = 1;
       volEnv1 = CalcPerStrikeVol(port, initialPrice, evalTime, riskFree, yield); 
    } else {
        volEnv1 = volEnv;
    }

    for(i = 0; i < iters; ++i) {
        RandStream_T random = RandStream_new();
        time_t etime = evalTime;
        price = initialPrice;

        for(d = 0; d < days; ++d) {
            double value = Contract_PortPrice(port, volCalc, volEnv1, etime, price, riskFree, vol); 
            payoffs[i * days + d] = value;

            price = Process_NextPrice(price, procKind, env, random);
            etime += 1 * 8 * 60 * 60;
        }
        RandStream_free(&random);
    }
    
    if(volsAssigned) {
        Table_T t = (Table_T) volEnv1;
        Table_map(t, freeVol, NULL);
        Table_free(&t);
    }

    return payoffs;
}

void MC_Save_Paths(unsigned iters, unsigned days, double* values, FILE* file) {
    unsigned i, d;

    for(d = 0; d < days; ++d) {
        for(i = 0; i < iters; ++i) {
       
            double v = values[i * days + d];
            fprintf(file, "%f\t", v);
        }
        fprintf(file, "\n");
    }
}