#ifndef LDERIV_INCLUDED
#define LDERIV_INCLUDED

#include <time.h>

#include "utils.h" // for BEGIN_DECLS
#include "portable.h" // for inline
#include "slist.h"
#include "randstream.h"

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
    double CostPerContract;
    Contract* Contract;
} Position;

typedef enum { Vol_ConstantForAllStrikes, Vol_ConstantByStrike } Contract_VolCalculation;

double Contract_Price(Contract* contract, time_t evalTime, double price, double riskFree, double vol);
double Contract_PortPrice(SList_T portfolio, Contract_VolCalculation volCalc, void* volEnv, time_t evalTime, double price, double riskFree, double vol); 

#define DAYSINYEAR 365.0
#define SECSINYEAR (DAYSINYEAR * 60 * 60 * 8) 

inline double diff_in_years(time_t end, time_t begin) {
   return difftime(end, begin) / SECSINYEAR;
}

typedef struct {
    double mean;
    double vol;
    double riskFree;
    double yield;
} Process_Brownian_Env;

typedef enum { Process_Brownian } Process_Kind;

double Process_NextPrice(double prevPrice, Process_Kind kind, void* env, RandStream_T random);

const char* Contract_ToAtom(Contract* contract);

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
                      unsigned days);

void MC_Save_Paths(unsigned iters, unsigned days, double* values, FILE* file);

END_DECLS

#endif
