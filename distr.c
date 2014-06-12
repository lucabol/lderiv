#include <math.h>

#ifndef Pi 
#define Pi 3.141592653589793238462643 
#endif 

// The cumulative normal distribution function 
static double cnd( double X )
{
    double L, K, w ;

    double const a1 = 0.31938153, a2 = -0.356563782, a3 = 1.781477937;
    double const a4 = -1.821255978, a5 = 1.330274429;

    L = fabs(X);
    K = 1.0 / (1.0 + 0.2316419 * L);
    w = 1.0 - 1.0 / sqrt(2 * Pi) * exp(-L *L / 2) * (a1 * K + a2 * K *K + a3 * pow(K,3) + a4 * pow(K,4) + a5 * pow(K,5));

    if (X < 0 ){
        w= 1.0 - w;
    }
    return w;
} 
