#include <math.h>

#include "lderiv.h"
#include "assert.h"

double Process_NextPrice(double prevPrice, Process_Kind kind, void* env) {

    switch(kind) {
        case Process_Brownian: {
            Process_Brownian_Env* envb = (Process_Brownian_Env*) env;
            double dailyDrift = (envb->mean + envb->riskFree - envb->yield - 0.5 * pow(envb->vol, 2.0)) * 1 / DAYSINYEAR;
            double dailyStDev = envb->vol / sqrt(DAYSINYEAR);
            double ret = dailyDrift + dailyStDev * RandStream_gauss(envb->rand, 1.0);
            return prevPrice * exp(ret);
            }
        default:
            assert(0.0);
            return 0.0;
    }
}
