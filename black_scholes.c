#include <math.h>

#include "assert.h"

double cnd(double);

#define SMALL_E 1e-100

double black_scholesG(char style, double s, double x, double t, double r, double b, double v) {
    double d1, d2;

    assert(v > 0);
    assert(s > 0);
    assert(x > 0);

    if(t < SMALL_E) t = SMALL_E;

    d1 = (log (s / x) + (b + v * v / 2.) * t) / (v * sqrt(t));
    d2 = d1 - v * sqrt(t);
    if(style == 'c') 
        return s * exp((b - r) * t) * cnd(d1) - x * exp(-r * t) * cnd(d2);
    else
        return x * exp(-r * t) * cnd(-d2) - s * exp((b - r) * t) * cnd(-d1);
}
