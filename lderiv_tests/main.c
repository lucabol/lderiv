#include <stdlib.h>

#include "test.h"
#include "mem.h"
#include "getopt.h"
#include "log.h"
#include "stats.h"
#include "atom.h"

#include "lderiv.h"

unsigned bs_test() {

    test_assert_float(16.0703, 0.01, black_scholesG('c', 102, 100, 2, 0.05, 0.05 - 0.03, 0.25));
    test_assert_float(10.4896, 0.01, black_scholesG('p', 102, 100, 2, 0.05, 0.05 - 0.03, 0.25));

    return TEST_SUCCESS;
}

unsigned contract_test() {
    time_t now = time(NULL);
    Contract_Underlying u = { "spx", 0.03, 0.25};
    Contract c = { &u, Contract_CallOption, 100, now + 30 * 8 * 60 * 60};
    double value = Contract_Price(&c, now, 100, 0.01, 0.25);
    test_assert_float(2.7727, 0.01, value);
    return TEST_SUCCESS;
}

unsigned contract_portfolio_test() {

    time_t now = time(NULL);
    Contract_Underlying u = { "spx", 0.03, 0.25};
    Contract c1 = { &u, Contract_CallOption, 100, now + 30 * 8 * 60 * 60};
    Position p1 = {1, 0.1, &c1};
    Contract c2 = { &u, Contract_CallOption, 100, now + 30 * 8 * 60 * 60};
    Position p2 = {1, 0.1, &c2};
    Contract c3 = {NULL, Contract_Commission, 0, 0};
    Position p3 = {-0.2, 0.1, &c3};
    Contract c4 = { &u, Contract_Stock, 0, 0};
    Position p4 = {3, 0.1, &c4};

    SList_T port = SList_list(&p1, &p2, &p3, &p4);
    double value = Contract_PortPrice(port, Vol_ConstantForAllStrikes, NULL, now, 100, 0.01, 0.25); 

    test_assert_float(2.7727 * 2 - 0.2 + 3 * 100, 0.01, value);

    SList_free(&port);
    return TEST_SUCCESS;
}

unsigned process_nextPrice_test() {

    int i, j, iters = 1000, days = 365;
    double price;
    RandStream_T random = RandStream_new();
    Process_Brownian_Env env = { 0.0, 0.3, 0.0, 0.0};
    Stats_T stats = Stats_New();

    for(i = 0; i < iters;++i) {
        price = 100;
        for(j = 0; j < days; ++j) {
            price = Process_NextPrice(price, Process_Brownian, &env, random);
        }
        Stats_Add(stats, price);
    }
    
    test_assert(stats->Max > 100);
    test_assert(stats->Min < 100);
    test_assert_float(100, 1.96 * stats->StdErr, stats->Average);

    RandStream_free(&random);
    Stats_Free(&stats);

    return TEST_SUCCESS;
}

static int days1 = 30;
static int price1 = 100;
static double yield1 = 0.03;
static double riskFree1 = 0.01;
static double vol1 = 0.25;


#define PRELUDE \
    int i, iters = 1000; \
    FILE* output; \
    Process_Brownian_Env env = { 0.0, vol1, riskFree1, yield1}; \
    Stats_T stats = Stats_New(); \
    time_t now = time(NULL); \
    double strike1 = 100, strike2 = 110, q1 = 1, q2 = 1; \
    time_t expiry = now + days1 * 8 * 60 * 60; \
    double daysInYears = diff_in_years(expiry, now); \
    double bs1 = black_scholesG('c', price1, strike1, daysInYears, riskFree1, yield1 - riskFree1, vol1); \
    double bs2 = black_scholesG('p', price1, strike2, daysInYears, riskFree1, yield1 - riskFree1, vol1); \
    double bsp1 = bs1 + 2, bsp2 = bs2 + 2; \
    Contract_Underlying u = { "spx", yield1, vol1}; \
    Contract c1 = { &u, Contract_CallOption, strike1, expiry}; \
    Position p1 = {q1, bsp1, &c1}; \
    Contract c2 = { &u, Contract_PutOption, strike2, expiry}; \
    Position p2 = {q2, bsp2, &c2}; \
    SList_T port = SList_list(&p1, &p2); \
    double bsPort = q1 * bs1 + q2 * bs2;

#define POSTLUDE \
    FREE(payoffs); \
    SList_free(&port); \
    Stats_Free(&stats); \
    return TEST_SUCCESS;

unsigned mc_runSinle_Varies_Test() {

    PRELUDE

    double* payoffs = MC_RunSingleT(port, Process_Brownian, &env, Vol_ConstantByStrike, NULL, price1, now, riskFree1, yield1, vol1, iters, days1);

    for( i = 0; i < iters ; ++i) {
        Stats_Add(stats, payoffs[i * days1]);
    }

    test_assert_float(q1 * bsp1 + q2 * bsp2, 0.1, stats->Average); // starts at price paid (as it keeps vols)

    Stats_Zero(stats);
    for( i = 0; i < iters ; ++i) {
        Stats_Add(stats, payoffs[i * days1 + days1 - 1]);
    }

    test_assert_float(bsPort, 1, stats->Average); // get close to black scholes price by the end

    // plot for [n=1:1000:10] "resultvaries.txt" using n with lines notitle, "" using (sum [n=1:1000] column(n))/ 1000 with linespoint title "Average" linewidth 2 linecolor rgb "red", "" using (13.1) with lines lw 4, "" using (2.77) with lines lw 4
    // plot "resultvaries.txt" using (sum [n=1:1000] column(n))/ 1000 with linespoint title "Average" linewidth 2 linecolor rgb "red", "" using (4.14) with lines lw 4, "" using (13.1) with lines lw 4
    output = fopen("resultvaries.txt", "w");
    MC_Save_Paths(iters, days1, payoffs, output);
    fclose(output);

    POSTLUDE
}

unsigned mc_runSingle_test() {
        
    PRELUDE
    int d;

    double* payoffs = MC_RunSingleT(port, Process_Brownian, &env, Vol_ConstantForAllStrikes, NULL, price1, now, riskFree1, yield1, vol1, iters, days1);

    for( i = 0; i < iters ; ++i) {
        Stats_Add(stats, payoffs[i * days1 + days1 - 1]);
    }

    test_assert_float(bsPort, 1.96 * stats->StdErr, stats->Average);

    for(d = 0; d < days1; ++d) {
        Stats_Zero(stats);
        for(i = 0; i < iters; ++i) {
            Stats_Add(stats, payoffs[i * days1 + d]);
        }
        test_assert_float(bsPort, bsPort * 0.1, stats->Average);
    }

    output = fopen("result.txt", "w");
    MC_Save_Paths(iters, days1, payoffs, output);
    fclose(output);
    // plot for [n=1:1000:10] "result.txt" using n with lines notitle, "" using (sum [n=1:1000] column(n))/ 1000 with linespoint title "Average" linewidth 2 linecolor rgb "red", "" using (13.10) with lines title "black scholes" linewidth 2 linecolor rgb "blue"
    // plot "result.txt" using (sum [n=1:1000] column(n))/ 1000 with linespoint title "Average" linewidth 2 linecolor rgb "red", "" using (13.10) with lines

    POSTLUDE
}

static int verbosity = 0;

static struct option long_options[] = {
    {"verbosity",      no_argument,       NULL,  'v' , "print out debug messages","", getopt_none, NULL ,&verbosity},
};

int main(int argc, char *argv[])
{
    if(getopt_parse(argc, argv, long_options, "-- tests for llib", "", "") < 0)
        exit(-3);

    if(verbosity)
        log_set(stderr, LOG_INFO);

    test_add("Black Scholes", "standard results", bs_test);
    test_add("Contract", "standard results", contract_test);
    test_add("Contract", "portfolio results", contract_portfolio_test);
    test_add("Process", "Brownian test", process_nextPrice_test);
    test_add("Montecarlo", "MC Single Thread", mc_runSingle_test);
    test_add("Montecarlo", "MC Single Thread varying", mc_runSinle_Varies_Test);
    test_run_all();

    Atom_freeAll();

    Mem_print_stats();

    return 0;
}
