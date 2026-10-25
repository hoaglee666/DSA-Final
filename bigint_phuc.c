#include "bigint.h"
#include <time.h>

// Deep copy a BigInt structure
BigInt *bigint_copy(const BigInt *src)
{
    if (!src)
        return NULL;
    BigInt *dst = create_bigint(src->capacity);
    dst->size = src->size;
    dst->sign = src->sign;
    memcpy(dst->digits, src->digits, src->size * sizeof(uint32_t));
    return dst;
}

// Check if a BigInt is exactly zero
int is_zero(const BigInt *a)
{
    return (!a || (a->size == 1 && a->digits[0] == 0));
}

// Single-limb division (fast short division path)
static void divmod_single_limb(const BigInt *u, uint32_t v_digit, BigInt **quotient, BigInt **remainder)
{
    BigInt *q = create_bigint(u->size);
    q->size = u->size;
    q->sign = u->sign;

    uint64_t rem = 0;
    for (int i = (int)u->size - 1; i >= 0; i--)
    {
        uint64_t cur = rem * BASE + u->digits[i];
        q->digits[i] = (uint32_t)(cur / v_digit);
        rem = cur % v_digit;
    }
    trim_leading_zeros(q);

    BigInt *r = int_to_bigint_internal((int64_t)rem);
    r->sign = (rem == 0) ? 1 : u->sign;

    *quotient = q;
    *remainder = r;
}

/*
 * Knuth's Algorithm D (The Art of Computer Programming, Vol 2, Sec 4.3.1)
 * Multi-precision non-negative integer division.
 * Divides u (dividend) by v (divisor) to produce quotient and remainder:
 *   u = quotient * v + remainder, where 0 <= remainder < |v|
 * Returns 0 on success, -1 on division by zero.
 */
int big_divmod(const BigInt *u, const BigInt *v, BigInt **quotient, BigInt **remainder)
{
    if (!u || !v || !quotient || !remainder)
        return -1;

    // Check division by zero
    if (is_zero(v))
    {
        *quotient = NULL;
        *remainder = NULL;
        return -1;
    }

    // Edge case 1: Dividend is zero
    if (is_zero(u))
    {
        *quotient = int_to_bigint_internal(0);
        *remainder = int_to_bigint_internal(0);
        return 0;
    }

    // Edge case 2: Compare absolute values
    int cmp = compare_abs(u, v);
    if (cmp < 0)
    {
        // |u| < |v|: quotient = 0, remainder = u
        *quotient = int_to_bigint_internal(0);
        *remainder = bigint_copy(u);
        return 0;
    }
    if (cmp == 0)
    {
        // |u| == |v|: quotient = sign(u) * sign(v), remainder = 0
        *quotient = int_to_bigint_internal((int64_t)(u->sign * v->sign));
        *remainder = int_to_bigint_internal(0);
        return 0;
    }

    // Edge case 3: Single-limb divisor
    if (v->size == 1)
    {
        divmod_single_limb(u, v->digits[0], quotient, remainder);
        (*quotient)->sign = u->sign * v->sign;
        return 0;
    }

    // Multi-limb division with Knuth's Algorithm D (n >= 2)
    size_t n = v->size;
    size_t m = u->size - n;

    // Step D1: Normalize
    // Choose d = floor(BASE / (v_{n-1} + 1))
    // This guarantees that the most significant limb of normalized divisor v' >= floor(BASE / 2)
    uint32_t vn_1 = v->digits[n - 1];
    uint32_t d = (uint32_t)(BASE / (vn_1 + 1));

    uint32_t *u_prime = (uint32_t *)calloc(m + n + 1, sizeof(uint32_t));
    uint32_t *v_prime = (uint32_t *)calloc(n, sizeof(uint32_t));
    uint32_t *p_scratch = (uint32_t *)calloc(n + 1, sizeof(uint32_t));
    if (!u_prime || !v_prime || !p_scratch)
    {
        free(u_prime);
        free(v_prime);
        free(p_scratch);
        return -1;
    }

    // Multiply v by d
    uint64_t carry = 0;
    for (size_t i = 0; i < n; i++)
    {
        uint64_t prod = (uint64_t)v->digits[i] * d + carry;
        v_prime[i] = (uint32_t)(prod % BASE);
        carry = prod / BASE;
    }

    // Multiply u by d
    carry = 0;
    for (size_t i = 0; i < u->size; i++)
    {
        uint64_t prod = (uint64_t)u->digits[i] * d + carry;
        u_prime[i] = (uint32_t)(prod % BASE);
        carry = prod / BASE;
    }
    u_prime[u->size] = (uint32_t)carry;

    // Allocate quotient
    BigInt *q = create_bigint(m + 1);
    q->size = m + 1;
    q->sign = u->sign * v->sign;

    // Step D2: Loop j from m down to 0
    for (int j = (int)m; j >= 0; j--)
    {
        // Step D3: Calculate q_hat (quotient estimate) and r_hat
        uint64_t num = (uint64_t)u_prime[j + n] * BASE + u_prime[j + n - 1];
        uint64_t q_hat, r_hat;

        if (u_prime[j + n] == v_prime[n - 1])
        {
            q_hat = BASE - 1;
            r_hat = (uint64_t)u_prime[j + n - 1] + v_prime[n - 1];
        }
        else
        {
            q_hat = num / v_prime[n - 1];
            r_hat = num % v_prime[n - 1];
        }

        // Refine q_hat: test condition q_hat * v_{n-2} > BASE * r_hat + u_{j+n-2}
        while (r_hat < BASE && (q_hat * v_prime[n - 2] > r_hat * BASE + u_prime[j + n - 2]))
        {
            q_hat--;
            r_hat += v_prime[n - 1];
        }

        // Step D4: Multiply and subtract
        // Compute P = q_hat * v_prime
        uint64_t mul_carry = 0;
        for (size_t i = 0; i < n; i++)
        {
            uint64_t prod = (uint64_t)q_hat * v_prime[i] + mul_carry;
            p_scratch[i] = (uint32_t)(prod % BASE);
            mul_carry = prod / BASE;
        }
        p_scratch[n] = (uint32_t)mul_carry;

        // Subtract P from u_prime[j .. j+n]
        int64_t sub_borrow = 0;
        for (size_t i = 0; i <= n; i++)
        {
            int64_t diff = (int64_t)u_prime[j + i] - p_scratch[i] - sub_borrow;
            if (diff < 0)
            {
                diff += BASE;
                sub_borrow = 1;
            }
            else
            {
                sub_borrow = 0;
            }
            u_prime[j + i] = (uint32_t)diff;
        }

        // Step D5 & D6: Test remainder and Add back if q_hat was 1 too big
        if (sub_borrow != 0)
        {
            q_hat--;
            uint64_t add_carry = 0;
            for (size_t i = 0; i < n; i++)
            {
                uint64_t sum = (uint64_t)u_prime[j + i] + v_prime[i] + add_carry;
                u_prime[j + i] = (uint32_t)(sum % BASE);
                add_carry = sum / BASE;
            }
            u_prime[j + n] = (uint32_t)((u_prime[j + n] + add_carry) % BASE);
        }

        q->digits[j] = (uint32_t)q_hat;
    }

    // Step D8: Unnormalize to get remainder
    // Remainder is in u_prime[0 .. n-1] divided by d
    BigInt *r = create_bigint(n);
    r->size = n;
    r->sign = u->sign;

    uint64_t unnorm_rem = 0;
    for (int i = (int)n - 1; i >= 0; i--)
    {
        uint64_t cur = unnorm_rem * BASE + u_prime[i];
        r->digits[i] = (uint32_t)(cur / d);
        unnorm_rem = cur % d;
    }

    trim_leading_zeros(q);
    trim_leading_zeros(r);

    if (is_zero(r))
        r->sign = 1;

    free(u_prime);
    free(v_prime);
    free(p_scratch);

    *quotient = q;
    *remainder = r;
    return 0;
}

// Division helper: returns quotient
BigInt *big_div(const BigInt *u, const BigInt *v)
{
    BigInt *q = NULL;
    BigInt *r = NULL;
    if (big_divmod(u, v, &q, &r) == 0)
    {
        free_bigint(r);
        return q;
    }
    return NULL;
}

// Modulo helper: returns remainder
BigInt *big_mod(const BigInt *u, const BigInt *v)
{
    BigInt *q = NULL;
    BigInt *r = NULL;
    if (big_divmod(u, v, &q, &r) == 0)
    {
        free_bigint(q);
        return r;
    }
    return NULL;
}

// ============================================================================
// UNIT TESTS & VERIFICATION
// ============================================================================

void run_phuc_tests(void)
{
    printf("\n=== RUNNING PHUC'S WEEK 2 UNIT TESTS ===\n");

    // 1. Week 1 Division Trace: 95432 / 187
    // Expected based on report: q = 510, r = 62 (510 * 187 + 62 = 95432)
    {
        printf("[Test 1] Week 1 Division Trace: 95432 / 187\n");
        BigInt *u = string_to_bigint("95432");
        BigInt *v = string_to_bigint("187");
        BigInt *q = NULL;
        BigInt *r = NULL;

        int status = big_divmod(u, v, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        printf("  Result:   q = %s, r = %s (status=%d)\n", q_buf, r_buf, status);
        printf("  Expected: q = 510, r = 62\n");
        if (strcmp(q_buf, "510") == 0 && strcmp(r_buf, "62") == 0 && status == 0)
            printf("  -> PASS!\n");
        else
            printf("  -> FAIL!\n");

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }

    // 2. Division by zero edge case
    {
        printf("[Test 2] Edge Case: Division by zero\n");
        BigInt *u = string_to_bigint("12345");
        BigInt *v = string_to_bigint("0");
        BigInt *q = NULL, *r = NULL;

        int status = big_divmod(u, v, &q, &r);
        if (status == -1 && q == NULL && r == NULL)
            printf("  -> PASS (Correctly rejected division by zero with error code -1)\n");
        else
            printf("  -> FAIL\n");

        free_bigint(u);
        free_bigint(v);
    }

    // 3. Dividend smaller than divisor (u < v)
    {
        printf("[Test 3] Edge Case: u < v (187 / 95432)\n");
        BigInt *u = string_to_bigint("187");
        BigInt *v = string_to_bigint("95432");
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        if (strcmp(q_buf, "0") == 0 && strcmp(r_buf, "187") == 0)
            printf("  -> PASS (q = 0, r = 187)\n");
        else
            printf("  -> FAIL (q = %s, r = %s)\n", q_buf, r_buf);

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }

    // 4. Dividend equals divisor (u == v)
    {
        printf("[Test 4] Edge Case: u == v (95432 / 95432)\n");
        BigInt *u = string_to_bigint("95432");
        BigInt *v = string_to_bigint("95432");
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        if (strcmp(q_buf, "1") == 0 && strcmp(r_buf, "0") == 0)
            printf("  -> PASS (q = 1, r = 0)\n");
        else
            printf("  -> FAIL (q = %s, r = %s)\n", q_buf, r_buf);

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }

    // 5. Divisor equals 1 (v == 1)
    {
        printf("[Test 5] Edge Case: Divisor is 1 (95432 / 1)\n");
        BigInt *u = string_to_bigint("95432");
        BigInt *v = string_to_bigint("1");
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        if (strcmp(q_buf, "95432") == 0 && strcmp(r_buf, "0") == 0)
            printf("  -> PASS (q = 95432, r = 0)\n");
        else
            printf("  -> FAIL (q = %s, r = %s)\n", q_buf, r_buf);

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }

    // 6. Multi-limb Knuth's Algorithm D test (n >= 2 limbs in base 10^4)
    {
        printf("[Test 6] Multi-limb Knuth Algorithm D: 123456789012345 / 12345678\n");
        BigInt *u = string_to_bigint("123456789012345");
        BigInt *v = string_to_bigint("12345678");
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        // 123456789012345 = 12345678 * 10000000 + 9012345 -> q = 10000000, r = 9012345
        printf("  Result:   q = %s, r = %s\n", q_buf, r_buf);
        printf("  Expected: q = 10000000, r = 9012345\n");
        if (strcmp(q_buf, "10000000") == 0 && strcmp(r_buf, "9012345") == 0)
            printf("  -> PASS!\n");
        else
            printf("  -> FAIL!\n");

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }

    // 7. Algorithm D Add-Back (Step D6) Activation Test
    // Constructed specifically so q_hat overshoots by 1 and triggers Step D6:
    // u = 1000000000000, v = 500000009999
    // Expected: q = 1, r = 499999990001
    {
        printf("[Test 7] Step D6 Add-Back Activation: 1000000000000 / 500000009999\n");
        BigInt *u = string_to_bigint("1000000000000");
        BigInt *v = string_to_bigint("500000009999");
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        printf("  Result:   q = %s, r = %s\n", q_buf, r_buf);
        printf("  Expected: q = 1, r = 499999990001\n");
        if (strcmp(q_buf, "1") == 0 && strcmp(r_buf, "499999990001") == 0)
            printf("  -> PASS (Add-back executed correctly!)\n");
        else
            printf("  -> FAIL!\n");

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }

    // 8. Negative division signs check
    {
        printf("[Test 8] Signed Division Semantics:\n");
        BigInt *u_neg = string_to_bigint("-95432");
        BigInt *v_pos = string_to_bigint("187");
        BigInt *q = NULL, *r = NULL;

        big_divmod(u_neg, v_pos, &q, &r);
        char q_buf[64], r_buf[64];
        bigint_to_string(q, q_buf, sizeof(q_buf));
        bigint_to_string(r, r_buf, sizeof(r_buf));

        // (-95432) / 187 -> q = -510, r = -62
        printf("  (-95432) / 187: q = %s, r = %s\n", q_buf, r_buf);
        if (strcmp(q_buf, "-510") == 0 && strcmp(r_buf, "-62") == 0)
            printf("  -> PASS!\n");
        else
            printf("  -> FAIL!\n");

        free_bigint(u_neg);
        free_bigint(v_pos);
        free_bigint(q);
        free_bigint(r);
    }

    printf("=== PHUC'S UNIT TESTS COMPLETED ===\n\n");
}

// ============================================================================
// ORACLE TESTING (Automated randomized validation against hardware arithmetic)
// ============================================================================

// Simple 64-bit pseudo-random generator
static uint64_t rng_state = 0x853c49e6748fea9bULL;
static uint64_t next_u64(void)
{
    rng_state ^= rng_state >> 12;
    rng_state ^= rng_state << 25;
    rng_state ^= rng_state >> 27;
    return rng_state * 0x2545F4914F6CDD1DULL;
}

// Generate random string of decimal digits
static void random_digit_string(char *buf, size_t num_digits)
{
    if (num_digits == 0)
    {
        strcpy(buf, "0");
        return;
    }
    buf[0] = '1' + (next_u64() % 9); // Non-zero leading digit
    for (size_t i = 1; i < num_digits; i++)
    {
        buf[i] = '0' + (next_u64() % 10);
    }
    buf[num_digits] = '\0';
}

void run_oracle_tests(void)
{
    printf("=== RUNNING AUTOMATED ORACLE TESTS ===\n");
    int passed = 0;
    int total = 0;

    // Oracle Test Suite 1: 64-bit Hardware Integer Comparison (500 iterations)
    printf("[Oracle Suite 1] Testing against native 64-bit hardware integer division (500 cases)...\n");
    for (int i = 0; i < 500; i++)
    {
        uint64_t a = (next_u64() >> 1); // Up to ~9 * 10^18
        uint64_t b = (next_u64() >> 1);
        if (b == 0)
            b = 1;
        if (a < b)
        {
            uint64_t tmp = a;
            a = b;
            b = tmp;
        }

        uint64_t exp_q = a / b;
        uint64_t exp_r = a % b;

        char a_str[32], b_str[32], exp_q_str[32], exp_r_str[32];
        snprintf(a_str, sizeof(a_str), "%llu", (unsigned long long)a);
        snprintf(b_str, sizeof(b_str), "%llu", (unsigned long long)b);
        snprintf(exp_q_str, sizeof(exp_q_str), "%llu", (unsigned long long)exp_q);
        snprintf(exp_r_str, sizeof(exp_r_str), "%llu", (unsigned long long)exp_r);

        BigInt *u = string_to_bigint(a_str);
        BigInt *v = string_to_bigint(b_str);
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);
        char act_q_str[64], act_r_str[64];
        bigint_to_string(q, act_q_str, sizeof(act_q_str));
        bigint_to_string(r, act_r_str, sizeof(act_r_str));

        total++;
        if (strcmp(act_q_str, exp_q_str) == 0 && strcmp(act_r_str, exp_r_str) == 0)
            passed++;
        else
        {
            printf("  FAIL on case %s / %s!\n", a_str, b_str);
            printf("    Got q=%s, r=%s\n", act_q_str, act_r_str);
            printf("    Exp q=%s, r=%s\n", exp_q_str, exp_r_str);
        }

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
    }
    printf("  -> Oracle 64-bit passed: %d / 500\n", passed);

    // Oracle Test Suite 2: Mathematical Invariant on Large Numbers (u = q * v + r, 0 <= r < v)
    // 200 cases with sizes from 20 to 120 digits
    printf("[Oracle Suite 2] Testing mathematical invariant (u = q * v + r, 0 <= r < v) on 200 large BigInts...\n");
    int inv_passed = 0;
    for (int i = 0; i < 200; i++)
    {
        size_t u_len = 20 + (next_u64() % 80); // 20 to 100 digits
        size_t v_len = 5 + (next_u64() % (u_len / 2 + 1));

        char u_buf[256], v_buf[256];
        random_digit_string(u_buf, u_len);
        random_digit_string(v_buf, v_len);

        BigInt *u = string_to_bigint(u_buf);
        BigInt *v = string_to_bigint(v_buf);
        BigInt *q = NULL, *r = NULL;

        big_divmod(u, v, &q, &r);

        // Check 1: 0 <= r < v
        int r_lt_v = (compare_abs(r, v) < 0);

        // Check 2: q * v + r == u
        BigInt *qv = bigint_mul(q, v);
        BigInt *reconstructed = bigint_add(qv, r);
        int rec_eq_u = (compare_abs(reconstructed, u) == 0);

        total++;
        if (r_lt_v && rec_eq_u)
        {
            passed++;
            inv_passed++;
        }
        else
        {
            printf("  FAIL on invariant case length u=%zu, v=%zu!\n", u_len, v_len);
            printf("    r < v: %d, rec == u: %d\n", r_lt_v, rec_eq_u);
        }

        free_bigint(u);
        free_bigint(v);
        free_bigint(q);
        free_bigint(r);
        free_bigint(qv);
        free_bigint(reconstructed);
    }
    printf("  -> Large Invariant passed: %d / 200\n", inv_passed);
    printf("=== ALL ORACLE TESTS COMPLETED: %d / %d PASSED (%.1f%%) ===\n\n",
           passed, total, (double)passed / total * 100.0);
}

// ============================================================================
// EMPIRICAL COMPLEXITY BENCHMARK
// ============================================================================

void run_division_benchmark(void)
{
    printf("=== EMPIRICAL COMPLEXITY ANALYSIS FOR KNUTH ALGORITHM D ===\n");
    printf("Measuring division runtime across scaling limb sizes:\n");
    printf("%-10s | %-12s | %-15s | %-15s | %-10s\n",
           "Limbs (N)", "Digits (~4N)", "Iterations", "Avg Time (us)", "T(N)/T(N/2)");
    printf("----------------------------------------------------------------------\n");

    int limb_sizes[] = {10, 20, 40, 80, 160, 320, 640};
    int num_sizes = sizeof(limb_sizes) / sizeof(limb_sizes[0]);
    double prev_time = 0.0;

    for (int s = 0; s < num_sizes; s++)
    {
        int N = limb_sizes[s];
        // Dividend of 2N limbs, Divisor of N limbs (m = N extra limbs, n = N divisor limbs)
        char *u_str = (char *)malloc(N * 2 * DIGIT_WIDTH + 1);
        char *v_str = (char *)malloc(N * DIGIT_WIDTH + 1);
        random_digit_string(u_str, N * 2 * DIGIT_WIDTH);
        random_digit_string(v_str, N * DIGIT_WIDTH);

        BigInt *u = string_to_bigint(u_str);
        BigInt *v = string_to_bigint(v_str);

        // Adjust iterations based on size to keep runtime responsive
        int iters = (N <= 40) ? 500 : (N <= 160) ? 100 : (N <= 320) ? 20 : 5;

        clock_t start = clock();
        for (int i = 0; i < iters; i++)
        {
            BigInt *q = NULL, *r = NULL;
            big_divmod(u, v, &q, &r);
            free_bigint(q);
            free_bigint(r);
        }
        clock_t end = clock();

        double elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
        double avg_us = (elapsed_sec / iters) * 1e6;

        char ratio_str[16] = "-";
        if (prev_time > 0.0)
        {
            snprintf(ratio_str, sizeof(ratio_str), "%.2fx", avg_us / prev_time);
        }

        printf("%-10d | %-12d | %-15d | %-15.2f | %-10s\n",
               N, N * DIGIT_WIDTH, iters, avg_us, ratio_str);

        prev_time = avg_us;

        free_bigint(u);
        free_bigint(v);
        free(u_str);
        free(v_str);
    }
    printf("----------------------------------------------------------------------\n");
    printf("Theoretical Knuth Algorithm D complexity: O(m * n).\n");
    printf("When doubling both m and n (N -> 2N), expected theoretical growth is 4.00x (O(N^2)).\n");
    printf("=== BENCHMARK COMPLETED ===\n\n");
}
