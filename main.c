#include "bigint.h"

int main(void)
{
    printf("=====================================================\n");
    printf("=== DSA FINAL PROJECT: BIGINT TEST & BENCHMARK SUITE \n");
    printf("=====================================================\n\n");

    // 1. Hoang's Unit Tests (Week 1 modexp trace: 7^13 mod 11)
    run_hoang_test();

    // 2. Phuc's Unit Tests (Week 1 division trace: 95432 / 187 and edge cases)
    run_phuc_tests();

    // 3. Phuc's Automated Oracle Tests (Hardware comparison & invariant verification)
    run_oracle_tests();

    // 4. Phuc's Empirical Complexity Benchmark for Division (Algorithm D)
    run_division_benchmark();

    printf("All test suites and benchmarks completed successfully.\n");
    return 0;
}