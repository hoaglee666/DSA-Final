#include "bigint.h"

int main(void)
{
    printf("=====================================================\n");
    printf("=== DSA FINAL PROJECT: BIGINT TEST & BENCHMARK SUITE \n");
    printf("=====================================================\n\n");

    run_hoang_test();
    run_modexp_benchmark();
    run_arithmetic_oracle_tests(1000);

    run_phuc_tests();
    run_oracle_tests();
    run_division_benchmark();

    printf("All test suites and benchmarks completed successfully.\n");
    return 0;
}