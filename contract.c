#include "lderiv.h"
#include "assert.h"
#include "mem.h"

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

struct envStruct {
    double total;
    time_t evalTime;
    double price;
    double riskFree;
    double vol;
};

static void cApply(void** x, void* cl) {
    Position* p = (Position*) *x;
    struct envStruct* e = (struct envStruct*) cl;

    e->total += Contract_Price(p->Contract, e->evalTime, e->price, e->riskFree, e->vol) * p->Quantity;
}

double Contract_PortPrice(SList_T portfolio, time_t evalTime, double price, double riskFree, double vol) {
    struct envStruct e = { 0.0, evalTime, price, riskFree, vol};
    SList_map(portfolio, cApply, &e);
    return e.total;
}