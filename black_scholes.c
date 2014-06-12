#include <math.h>

#include "assert.h"

double cnd(double);

#define SMALL_E 1e-100

// style: either Call or Put
// s: stock price
// x: strike price of option
// t: time to expiration in years
// r: risk free interest rate
// v: volatility
// b: cost of carry
//   b = r for B & S (1973) European no dividend
//   b = r - y Merton (1973) European stock option with continuous dividend
//   b = 0     Black (1976) Future option model
//   b = 0 r = 0 Asay (1982) margined future option model
//   b = r - rf Garman & Kohlhagen (1983) currency option model
double black_scholesG(char style, double s, double x, double t, double r, double b, double v) {
    double d1, d2;

    assert(v > 0);
    assert(s > 0);
    assert(x > 0);

    if(t < SMALL_E) t = SMALL_E;

    d1 = (log (s / x) + (b + v * v / 2.) * t) / (v * sqrt(t));
    d2 = d1 - v * sqrt(t);
    if(style = 'c') 
        return s * exp((b - r) * t) * cnd(d1) - x * exp(-r * t) * cnd(d2);
    else
        return x * exp(-r * t) * cnd(-d2) - s * exp((b - r) * t) * cnd(-d1);
}
