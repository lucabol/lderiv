#ifndef LDERIV_INCLUDED
#define LDERIV_INCLUDED

#include <time.h>

#include "utils.h" // for BEGIN_DECLS
#include "portable.h" // for inline
#include "slist.h"

BEGIN_DECLS

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
extern double black_scholesG(char style, double s, double x, double t, double r, double b, double v);

typedef enum { Contract_Stock, Contract_CallOption, Contract_PutOption, Contract_Commission} Contract_Kind;

typedef struct {
    char ticker[10];
    double Yield;
    double Volatility;
} Contract_Underlying;

typedef struct {
    Contract_Underlying* Underlying;
    Contract_Kind Kind;
    double Strike;
    time_t Expiry;
} Contract;

typedef struct {
    double Quantity;
    Contract* Contract;
} Position;

double Contract_Price(Contract* contract, time_t evalTime, double price, double riskFree, double vol);
double Contract_PortPrice(SList_T portfolio, time_t evalTime, double price, double riskFree, double vol); 

#define DAYSINYEAR 365
#define SECSINYEAR (DAYSINYEAR * 60 * 60 * 8) 

inline double diff_in_years(time_t end, time_t begin) {
   return difftime(end, begin) / SECSINYEAR;
}

END_DECLS

#endif
