#include "bigint.h"
#include <time.h>

// allo new bigint with dynamic memo
BigInt *create_bigint(size_t init_cap)
{
    if (init_cap == 0)
        init_cap = 4;
    BigInt *num = (BigInt *)malloc(sizeof(BigInt));
    if (!num)
    {
        perror("ALLOCATION FAILED");
        exit(EXIT_FAILURE);
    }
    num->digits = (uint32_t *)calloc(init_cap, sizeof(uint32_t));
    if (!num->digits)
    {
        perror("ALLOCATION FAILED");
        exit(EXIT_FAILURE);
    }
    num->size = 1;
    num->capacity = init_cap;
    num->sign = 1;
    return num;
}

// free mem l6-7
void free_bigint(BigInt *num)
{
    if (num)
    {
        if (num->digits)
            free(num->digits);
        free(num);
    }
}

// remove redun lead zero
void trim_leading_zeros(BigInt *num)
{
    while (num->size > 1 && num->digits[num->size - 1] == 0)
    {
        num->size--;
    }
    if (num->size == 1 && num->digits[0] == 0)
    {
        num->sign = 1;
    }
}

BigInt *int_to_bigint_internal(int64_t val)
{
    BigInt *num = create_bigint(4);
    if (val < 0)
    {
        num->sign = -1;
        val = -val;
    }
    else
        num->sign = 1;

    if (val == 0)
    {
        num->digits[0] = 0;
        num->size = 1;
        return num;
    }

    size_t idx = 0;
    while (val > 0)
    {
        if (idx >= num->capacity)
        {
            num->capacity *= 2;
            num->digits = (uint32_t *)realloc(num->digits, num->capacity * sizeof(uint32_t));
        }
        num->digits[idx++] = val % BASE;
        val /= BASE;
    }
    num->size = idx;
    return num;
}

// stri to bigint, right to left
BigInt *string_to_bigint(const char *str)
{
    if (!str || *str == '\0')
        return int_to_bigint_internal(0);

    int sign = 1;
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
        str++;

    size_t len = strlen(str);
    size_t est_limbs = (len + DIGIT_WIDTH - 1) / DIGIT_WIDTH;
    BigInt *num = create_bigint(est_limbs);
    num->sign = sign;
    num->size = 0;

    // parse right to left
    int end = (int)len;
    while (end > 0)
    {
        int start = end - DIGIT_WIDTH;
        if (start < 0)
            start = 0;

        uint32_t chunk_val = 0;
        for (int i = start; i < end; i++)
        {
            chunk_val = chunk_val * 10 + (str[i] - '0');
        }

        if (num->size >= num->capacity)
        {
            num->capacity *= 2;
            num->digits = (uint32_t *)realloc(num->digits, num->capacity * sizeof(uint32_t));
        }
        num->digits[num->size++] = chunk_val;
        end = start;
    }
    trim_leading_zeros(num);
    return num;
}

// format bigint to stri buff
void bigint_to_string(const BigInt *num, char *buffer, size_t max_len)
{
    if (max_len == 0)
        return;
    size_t pos = 0;

    if (num->sign == -1 && !(num->size == 1 && num->digits[0] == 0))
    {
        if (pos < max_len - 1)
            buffer[pos++] = '-';
    }

    int top = (int)num->size - 1;
    int written = snprintf(buffer + pos, max_len - pos, "%u", num->digits[top]);
    pos += written;

    for (int i = top - 1; i >= 0; i--)
    {
        written = snprintf(buffer + pos, max_len - pos, "%04u", num->digits[i]);
        pos += written;
        if (pos >= max_len - 1)
            break;
    }
    buffer[pos] = '\0';
}

// abs val comp, 1 if a > b, -1 if b > a
int compare_abs(const BigInt *a, const BigInt *b)
{
    if (a->size != b->size)
    {
        return (a->size > b->size) ? 1 : -1;
    }

    for (int i = (int)a->size - 1; i >= 0; i--)
    {
        if (a->digits[i] != b->digits[i])
        {
            return (a->digits[i] > b->digits[i]) ? 1 : -1;
        }
    }
    return 0;
}

// add
BigInt *bigint_add(const BigInt *a, const BigInt *b)
{
    if (a->sign == b->sign)
    {
        size_t max_len = (a->size > b->size) ? a->size : b->size;
        BigInt *res = create_bigint(max_len + 1);
        uint64_t carry = 0;

        for (size_t i = 0; i < max_len || carry; i++)
        {
            if (i >= res->capacity)
            {
                res->capacity *= 2;
                res->digits = (uint32_t *)realloc(res->digits, res->capacity * sizeof(uint32_t));
            }
            uint64_t sum = carry;
            if (i < a->size)
                sum += a->digits[i];
            if (i < b->size)
                sum += b->digits[i];

            res->digits[i] = sum % BASE;
            carry = sum / BASE;
            res->size = i + 1;
        }
        res->sign = a->sign;
        trim_leading_zeros(res);
        return res;
    }
    else
    {
        int cmp = compare_abs(a, b);
        if (cmp == 0)
        {
            BigInt *res = create_bigint(1);
            res->digits[0] = 0;
            res->size = 1;
            res->sign = 1;
            return res;
        }

        const BigInt *larger = (cmp > 0) ? a : b;
        const BigInt *smaller = (cmp > 0) ? b : a;

        BigInt *res = create_bigint(larger->size);
        int64_t borrow = 0;

        for (size_t i = 0; i < larger->size; i++)
        {
            int64_t diff = (int64_t)larger->digits[i] - borrow;
            if (i < smaller->size)
            {
                diff -= smaller->digits[i];
            }
            if (diff < 0)
            {
                diff += BASE;
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            res->digits[i] = (uint32_t)diff;
            res->size = i + 1;
        }
        res->sign = (cmp > 0) ? a->sign : b->sign;
        trim_leading_zeros(res);
        return res;
    }
}

// sub
BigInt *bigint_sub(const BigInt *a, const BigInt *b)
{
    BigInt *neg_b = create_bigint(b->size);
    neg_b->size = b->size;
    neg_b->sign = -b->sign;

    for (size_t i = 0; i < b->size; i++)
    {
        neg_b->digits[i] = b->digits[i];
    }
    BigInt *res = bigint_add(a, neg_b);
    free_bigint(neg_b);
    return res;
}

// mul
BigInt *bigint_mul(const BigInt *a, const BigInt *b)
{
    BigInt *res = create_bigint(a->size + b->size);
    res->size = a->size + b->size;
    for (size_t i = 0; i < res->size; i++)
        res->digits[i] = 0;

    for (size_t i = 0; i < a->size; i++)
    {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->size || carry; j++)
        {
            uint64_t product = res->digits[i + j] + carry;
            if (j < b->size)
                product += (uint64_t)a->digits[i] * b->digits[j];
            res->digits[i + j] = product % BASE;
            carry = product / BASE;
        }
    }
    res->sign = (a->sign == b->sign) ? 1 : -1;
    trim_leading_zeros(res);
    return res;
}

// naive
BigInt *big_modexp_naive(const BigInt *base, const BigInt *exp, const BigInt *mod, int verbose)
{
    BigInt *res = int_to_bigint_internal(1);
    BigInt *cur = bigint_copy(base);
    if (mod)
    {
        BigInt *reduced = big_mod(cur, mod);
        free_bigint(cur);
        cur = reduced;
    }

    // extract scalar
    uint64_t e = 0;
    for (int i = (int)exp->size - 1; i >= 0; i--)
        e = e * BASE + exp->digits[i];

    char buf_res[128], buf_cur[128], buf_prod[128];
    bigint_to_string(cur, buf_cur, sizeof(buf_cur));
    if (verbose)
        printf("[NAIVE MODEXP] BASE b = %s, EXPONENT e = %lu, INITIAL ACCUMULATOR r = 1\n", buf_cur, e);

    for (uint64_t i = 1; i <= e; i++)
    {
        bigint_to_string(res, buf_res, sizeof(buf_res));
        BigInt *temp = bigint_mul(res, cur);
        bigint_to_string(temp, buf_prod, sizeof(buf_prod));

        free_bigint(res);
        if (mod)
        {
            res = big_mod(temp, mod);
            free_bigint(temp);
        }
        else
        {
            res = temp;
        }
        if (verbose)
        {
            bigint_to_string(res, buf_res, sizeof(buf_res));
            printf("-> NAIVE STEP %lu/%lu: r * b = %s -> MOD REDUCTION -> NEW r = %s\n", i, e, buf_prod, buf_res);
        }
    }
    free_bigint(cur);
    return res;
}

// square and mul
BigInt *big_modexp(const BigInt *base, const BigInt *exp, const BigInt *mod, int verbose)
{
    BigInt *res = int_to_bigint_internal(1);
    BigInt *cur = bigint_copy(base);
    if (mod)
    {
        BigInt *reduced = big_mod(cur, mod);
        free_bigint(cur);
        cur = reduced;
    }

    uint64_t e = 0;
    for (int i = (int)exp->size - 1; i >= 0; i--)
        e = e * BASE + exp->digits[i];

    char buf_res[128], buf_cur[128], buf_sq[128], buf_mult[128];
    bigint_to_string(cur, buf_cur, sizeof(buf_cur));

    // binary conv
    char bin_buf[128];
    int bin_idx = 0;
    uint64_t temp_e_bin = e;
    int bit_len = 0;

    if (temp_e_bin == 0)
    {
        bin_buf[bin_idx++] = '0';
        bit_len = 1;
    }
    else
    {
        int hi = 63;
        while (hi >= 0 && !((temp_e_bin >> hi) & 1))
        {
            hi--;
        }
        for (int i = hi; i >= 0; i--)
        {
            bin_buf[bin_idx++] = ((temp_e_bin >> i) & 1) ? '1' : '0';
            bit_len++;
        }
    }
    bin_buf[bin_idx] = '\0';

    if (verbose)
    {
        printf("[FAST MODEXP] EXPONENT %lu IN BINARY is %s_2 (%d bits)\n", e, bin_buf, bit_len);
        printf("INITIAL STATE: r = 1, base b = %s\n", buf_cur);
    }

    // msb-lsb
    int highest_bit = -1;
    for (int b_idx = 63; b_idx >= 0; b_idx--)
    {
        if ((e >> b_idx) & 1)
        {
            highest_bit = b_idx;
            break;
        }
    }

    int step = 1;
    for (int b_idx = highest_bit; b_idx >= 0; b_idx--)
    {
        int bit = (e >> b_idx) & 1;
        int is_msb = (b_idx == highest_bit) ? 1 : 0;

        bigint_to_string(res, buf_res, sizeof(buf_res));
        if (verbose)
            printf("-> STEP %d (BIT = %d%s): ", step++, bit, is_msb ? ", MSB" : "");

        // square opera: r^2 mod m
        BigInt *squared_res = bigint_mul(res, res);
        BigInt *mod_squared_res = mod ? big_mod(squared_res, mod) : bigint_copy(squared_res);
        free_bigint(squared_res);
        bigint_to_string(mod_squared_res, buf_sq, sizeof(buf_sq));

        if (bit)
        {
            // snm by base
            BigInt *mult = bigint_mul(mod_squared_res, cur);
            BigInt *mod_mult = mod ? big_mod(mult, mod) : bigint_copy(mult);
            free_bigint(mult);
            free_bigint(mod_squared_res); // can free bc modsq isnt assign to res

            bigint_to_string(mod_mult, buf_mult, sizeof(buf_mult));
            if (verbose)
                printf("SQUARE r^2 mod m = %s; MULTIPLY BY BASE -> %s mod m\n", buf_sq, buf_mult);

            free_bigint(res);
            res = mod_mult;
        }
        else
        {
            // bit = 0
            if (verbose)
                printf("SQUARE r^2 mod m = %s (MULTIPLY SKIPPED)\n", buf_sq);

            free_bigint(res);
            res = mod_squared_res;
        }

        bigint_to_string(res, buf_res, sizeof(buf_res));
        if (verbose)
            printf("UPDATED ACCUMULATOR -> r = %s\n", buf_res);
    }

    free_bigint(cur);
    return res;
}

void run_modexp_benchmark(void)
{
    printf("\n=== FAST VS. NAIVE MODEXP BENCHMARK ===\n");

    // base = 123, mod = 10007, incre exp
    BigInt *base = string_to_bigint("123");
    BigInt *mod = string_to_bigint("10007");

    const char *test_exps[] = {"10", "100", "500", "1000", "5000", "10000"};
    int num_tests = sizeof(test_exps) / sizeof(test_exps[0]);

    printf("%-10s | %-18s | %-18s | %-12s\n", "EXPONENT", "FAST MODEXP (ms)", "NAIVE MODEXP (ms)", "SPEEDUP RATIO");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < num_tests; i++)
    {
        BigInt *exp = string_to_bigint(test_exps[i]);

        // bench fast modexp
        clock_t start_f = clock();
        BigInt *res_f = big_modexp(base, exp, mod, 0);
        clock_t end_f = clock();
        double time_f = ((double)(end_f - start_f)) * 1000.0 / CLOCKS_PER_SEC;

        // bench naive (skip if exp 2 large, prevent hanging)
        // e = 5000 = slow in base10^4
        double time_n = 0.0;
        if (i <= 4)
        {
            clock_t start_n = clock();
            BigInt *res_n = big_modexp_naive(base, exp, mod, 0);
            clock_t end_n = clock();
            time_n = ((double)(end_n - start_n)) * 1000.0 / CLOCKS_PER_SEC;
            free_bigint(res_n);
        }

        if (time_n > 0.0)
        {
            double speedup = time_n / (time_f > 0.0 ? time_f : 0.001);
            printf("%-10s | %-18.3f | %-18.3f | %-8.2f\n", test_exps[i], time_f, time_n, speedup);
        }
        else
        {
            printf("%-10s | %-18.3f | %-18s | %-12s\n", test_exps[i], time_f, "TIMEOUT (>5s)", "N/A (O(e))");
        }

        free_bigint(exp);
        free_bigint(res_f);
    }

    free_bigint(base);
    free_bigint(mod);
    printf("=== MODEXP BENCHMARK COMPLETED ===\n");
}

void run_arithmetic_oracle_tests(int num_iterations)
{
    printf("\n=== ARITHMETIC ORACLE TESTING (vs. __int128) ===\n");
    int add_passed = 0;
    int sub_passed = 0;
    int mul_passed = 0;

    for (int i = 0; i < num_iterations; i++)
    {
        int64_t val1 = (rand() % 2000000000ULL) - 1000000000ULL;
        int64_t val2 = (rand() % 2000000000ULL) - 1000000000ULL;

        char buf1[64], buf2[64];
        snprintf(buf1, sizeof(buf1), "%lld", (long long)val1);
        snprintf(buf2, sizeof(buf2), "%lld", (long long)val2);

        BigInt *b1 = string_to_bigint(buf1);
        BigInt *b2 = string_to_bigint(buf2);

        // --- 1. TEST ADDITION ---
        BigInt *b_sum = bigint_add(b1, b2);
        char buf_sum[128];
        bigint_to_string(b_sum, buf_sum, sizeof(buf_sum));

        int64_t expected_sum = val1 + val2;
        char exp_sum_buff[128];
        snprintf(exp_sum_buff, sizeof(exp_sum_buff), "%lld", (long long)expected_sum);

        if (strcmp(buf_sum, exp_sum_buff) == 0)
        {
            add_passed++;
        }
        else
        {
            printf("ADD MISMATCH! %lld + %lld -> GOT %s, EXPECTED %s\n",
                   (long long)val1, (long long)val2, buf_sum, exp_sum_buff);
        }

        // --- 2. TEST SUBTRACTION ---
        BigInt *b_sub = bigint_sub(b1, b2);
        char buf_sub[128];
        bigint_to_string(b_sub, buf_sub, sizeof(buf_sub));

        int64_t expected_sub = val1 - val2;
        char ex_sub_buff[128];
        snprintf(ex_sub_buff, sizeof(ex_sub_buff), "%lld", (long long)expected_sub);

        if (strcmp(buf_sub, ex_sub_buff) == 0)
        {
            sub_passed++;
        }
        else
        {
            printf("SUB MISMATCH! %lld - %lld -> GOT %s, EXPECTED %s\n",
                   (long long)val1, (long long)val2, buf_sub, ex_sub_buff);
        }

        // --- 3. TEST MULTIPLICATION ---
        BigInt *b_prod = bigint_mul(b1, b2);
        char buf_prod[128];
        bigint_to_string(b_prod, buf_prod, sizeof(buf_prod));

        // Avoid sign-extension issues with safe absolute value casting
        unsigned __int128 abs_v1 = (val1 < 0) ? (unsigned __int128)(-val1) : (unsigned __int128)val1;
        unsigned __int128 abs_v2 = (val2 < 0) ? (unsigned __int128)(-val2) : (unsigned __int128)val2;
        unsigned __int128 expected_prod = abs_v1 * abs_v2;
        int is_negative = (val1 < 0) ^ (val2 < 0);
        if (expected_prod == 0)
            is_negative = 0;

        // Build expected string for __int128 product
        char expected_buf[128];
        int idx = 0;
        if (is_negative)
            expected_buf[idx++] = '-';

        unsigned __int128 temp_val = expected_prod;
        if (temp_val == 0)
        {
            expected_buf[idx++] = '0';
        }
        else
        {
            char rev[128];
            int r_idx = 0;
            while (temp_val > 0)
            {
                rev[r_idx++] = '0' + (int)(temp_val % 10);
                temp_val /= 10;
            }
            for (int r = r_idx - 1; r >= 0; r--)
            {
                expected_buf[idx++] = rev[r];
            }
        }
        expected_buf[idx] = '\0';

        if (strcmp(buf_prod, expected_buf) == 0)
        {
            mul_passed++;
        }
        else
        {
            printf("MUL MISMATCH! %lld * %lld -> GOT %s, EXPECTED %s\n",
                   (long long)val1, (long long)val2, buf_prod, expected_buf);
        }

        // Clean up iteration objects
        free_bigint(b1);
        free_bigint(b2);
        free_bigint(b_sum);
        free_bigint(b_sub);
        free_bigint(b_prod);
    }

    printf("Oracle Results:\n");
    printf("  - Addition:    %d / %d passed\n", add_passed, num_iterations);
    printf("  - Subtraction: %d / %d passed\n", sub_passed, num_iterations);
    printf("  - Multiplication: %d / %d passed\n", mul_passed, num_iterations);
    printf("=== ORACLE TESTING COMPLETED ===\n");
}

void run_hoang_test()
{
    printf("=== RUNNING HOANG UNIT TESTS ===\n");
    printf("\n1. ADDITION & SUBTRACTION TESTING\n");

    char buf[128];

    const char *e1 = "-987654321";
    const char *e2 = "-123456789";
    const char *ex = "2375193652";

    BigInt *s = string_to_bigint(e1);
    BigInt *d = string_to_bigint(e2);

    BigInt *sum = bigint_add(s, d);
    BigInt *diff = bigint_sub(s, d);

    bigint_to_string(sum, buf, sizeof(buf));
    printf("%s + %s = %s\n", e1, e2, buf);
    bigint_to_string(diff, buf, sizeof(buf));
    printf("%s - %s = %s\n", e1, e2, buf);

    printf("\n2. MULTIPLICATION TESTING\n");

    const char *e3 = "1234";
    const char *e4 = "5678";

    BigInt *m1 = string_to_bigint(e3);
    BigInt *m2 = string_to_bigint(e4);
    BigInt *prod = bigint_mul(m1, m2);
    bigint_to_string(prod, buf, sizeof(buf));
    printf("%s * %s = %s\n", e3, e4, buf);

    printf("\n3. ModExp TESTING\n");
    const char *eb = "7";
    const char *ee = "0";
    const char *em = "11";

    BigInt *b = string_to_bigint(eb);
    BigInt *e = string_to_bigint(ee);
    BigInt *m = string_to_bigint(em);

    printf("3.1. FAST MODULAR EXPONENTIATION TESTING\n");
    BigInt *ans_fast = big_modexp(b, e, m, 1);
    bigint_to_string(ans_fast, buf, sizeof(buf));
    printf("FAST MODEXP %s^%s mod %s -> RESULT: %s\n", eb, ee, em, buf);

    printf("3.2. NAIVE MODULAR EXPONENTIATION TESTING\n");
    BigInt *ans_naive = big_modexp_naive(b, e, m, 1);
    bigint_to_string(ans_naive, buf, sizeof(buf));
    printf("NAIVE MODEXP %s^%s mod %s -> RESULT: %s\n", eb, ee, em, buf);

    free_bigint(s);
    free_bigint(d);
    free_bigint(sum);
    free_bigint(diff);

    free_bigint(m1);
    free_bigint(m2);
    free_bigint(prod);

    free_bigint(b);
    free_bigint(e);
    free_bigint(m);
    free_bigint(ans_fast);
    free_bigint(ans_naive);

    printf("=== HOANG'S UNIT TESTS COMPLETED ===\n");
}