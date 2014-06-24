#include <stdlib.h>

#include "test.h"
#include "mem.h"
#include "getopt.h"
#include "log.h"
#include "stats.h"

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
    Position p1 = {1, &c1};
    Contract c2 = { &u, Contract_CallOption, 100, now + 30 * 8 * 60 * 60};
    Position p2 = {1, &c2};
    Contract c3 = {NULL, Contract_Commission, 0, 0};
    Position p3 = {-0.2, &c3};
    Contract c4 = { &u, Contract_Stock, 0, 0};
    Position p4 = {3, &c4};

    SList_T port = SList_list(&p1, &p2, &p3, &p4, NULL);
    double value = Contract_PortPrice(port, now, 100, 0.01, 0.25); 

    test_assert_float(2.7727 * 2 - 0.2 + 3 * 100, 0.01, value);

    SList_free(&port);
    return TEST_SUCCESS;
}

unsigned process_nextPrice_test() {

    int i, j, iters = 1000, days = 365;
    double price;
    RandStream_T rand = RandStream_new();
    Process_Brownian_Env env = { 0.0, 0.3, 0.0, 0.0, rand };
    Stats_T stats = Stats_New();

    for(i = 0; i < iters;++i) {
        price = 100;
        for(j = 0; j < days; ++j) {
            price = Process_NextPrice(price, Process_Brownian, &env);
        }
        Stats_Add(stats, price);
    }
    
    test_assert(stats->Max > 100);
    test_assert(stats->Min < 100);
    test_assert_float(100, 1.96 * stats->StdErr, stats->Average);

    RandStream_free(&rand);
    Stats_Free(&stats);

    return TEST_SUCCESS;
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
    test_run_all();
    Mem_print_stats();

    return 0;
}
