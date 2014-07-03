#include "lderiv.h"
#include "assert.h"
#include "mem.h"
#include "table.h"

double Contract_Price(Contract* contract, time_t evalTime, double price, double riskFree, double vol) {

    switch(contract->Kind) {
        case Contract_Stock:
            return price;
        case Contract_CallOption:
            return black_scholesG('c', price, contract->Strike, diff_in_years(contract->Expiry, evalTime), riskFree, riskFree - contract->Underlying->Yield, vol); 
        case Contract_PutOption:
            return black_scholesG('p', price, contract->Strike, diff_in_years(contract->Expiry, evalTime), riskFree, riskFree - contract->Underlying->Yield, vol); 
        case Contract_Commission:
            return 1.0;
        default:
            assert(0.0);
            return 0.0;
    }
}

struct applyStruct {
    double total;
    time_t evalTime;
    double price;
    double riskFree;
    double vol;
    Table_T volTable;
};

static void cApply(void** x, void* cl) {
    Position* p = (Position*) *x;
    struct applyStruct* e = (struct applyStruct*) cl;
    double vol = e->vol;

    if(e->volTable)
        vol = * (double*) Table_get(e->volTable, p->Contract);

    e->total += Contract_Price(p->Contract, e->evalTime, e->price, e->riskFree, vol) * p->Quantity;
}

double Contract_PortPrice(SList_T portfolio, Contract_VolCalculation volCalc, void* volEnv, time_t evalTime, double price, double riskFree, double vol) {
    struct applyStruct e = { 0.0, evalTime, price, riskFree, vol, NULL};

    if(volCalc == Vol_ConstantByStrike && volEnv) {
        e.volTable = (Table_T) volEnv;
    }

    SList_map(portfolio, cApply, &e);
    return e.total;
}