#include "bigint.h"

// allo new bigint with dynamic memo
BigInt *create_bigint(size_t init_cap)
{
    if (init_cap == 0)
        init_cap = 4;
    BigInt *num = (BigInt *)malloc(sizeof(BigInt));
    if (!num)
    {
        perror("allocation failed");
        exit(EXIT_FAILURE);
    }
    num->digits = (uint32_t *)calloc(init_cap, sizeof(uint32_t));
    if (!num->digits)
    {
        perror("allocation failed");
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
    trim_leading_zeros(res);
    return res;
}

// sub
BigInt *bigint_sub(const BigInt *a, const BigInt *b)
{
    BigInt *res = create_bigint(a->size);
    int64_t borrow = 0;

    for (size_t i = 0; i < a->size; i++)
    {
        int64_t diff = (int64_t)a->digits[i] - borrow;
        if (i < b->size)
            diff -= b->digits[i];
        if (diff < 0)
        {
            diff += BASE;
            borrow = 1;
        }
        else
            borrow = 0;
        res->digits[i] = (uint32_t)diff;
        res->size = i + 1;
    }
    trim_leading_zeros(res);
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
    res->sign = a->sign & b->sign;
    trim_leading_zeros(res);
    return res;
}

// naive
BigInt *big_modexp_naive(const BigInt *base, const BigInt *exp, const BigInt *mod)
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
    printf("[NAIVE TRACE START] BASE b = %s, EXPONENT e = %lu, INITIAL ACCUMULATOR r = 1\n", buf_cur, e);

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

        bigint_to_string(res, buf_res, sizeof(buf_res));
        printf("-> NAIVE STEP %lu/%lu: r * b = %s -> MOD REDUCTION -> NEW r = %s\n", i, e, buf_prod, buf_res);
    }
    free_bigint(cur);
    return res;
}

// square and mul
BigInt *big_modexp(const BigInt *base, const BigInt *exp, const BigInt *mod)
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

    printf("[FAST MODEXP] EXPONENT %lu IN BINARY is %s_2 (%d bits)\n", e, bin_buf, bit_len);
    printf("INITIAL STATE: r = 1, base b = %s\n", buf_cur);

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
            printf("SQUARE r^2 mod m = %s; MULTIPLY BY BASE -> %s mod m\n", buf_sq, buf_mult);
            free_bigint(res);
            res = mod_mult;
        }
        else
        {
            // bit = 0
            printf("SQUARE r^2 mod m = %s (MULTIPLY SKIPPED)\n", buf_sq);
            free_bigint(res);
            res = mod_squared_res;
        }

        bigint_to_string(res, buf_res, sizeof(buf_res));
        printf("UPDATED ACCUMULATOR -> r = %s\n", buf_res);
    }

    free_bigint(cur);
    return res;
}

void run_hoang_test()
{
    printf("=== RUNNING HOANG UNIT TESTS ===\n");
    printf("\n1. ADDITION & SUBTRACTION TESTING\n");

    char buf[128];

    BigInt *a = string_to_bigint("987654321");
    BigInt *b = string_to_bigint("123456789");
    BigInt *sum = bigint_add(a, b);
    BigInt *diff = bigint_sub(a, b);

    bigint_to_string(sum, buf, sizeof(buf));
    printf("987654321 + 123456789 = %s (EXPECTED 1111111110)\n", buf);
    bigint_to_string(diff, buf, sizeof(buf));
    printf("987654321 - 123456789 = %s (EXPECTED 864197532)\n", buf);

    printf("\n2. MULTIPLICATION TESTING\n");
    BigInt *m1 = string_to_bigint("1234");
    BigInt *m2 = string_to_bigint("5678");
    BigInt *prod = bigint_mul(m1, m2);
    bigint_to_string(prod, buf, sizeof(buf));
    printf("1234 * 5678 = %s (EXPECTED 7006652)\n", buf);

    printf("\n3. ModExp TESTING\n");
    BigInt *b7 = string_to_bigint("7");
    BigInt *e13 = string_to_bigint("13");
    BigInt *m11 = string_to_bigint("11");

    printf("\n-3.1. FAST MODULAR EXPONENTIATION TESTING\n");
    BigInt *ans_fast = big_modexp(b7, e13, m11);
    bigint_to_string(ans_fast, buf, sizeof(buf));
    printf("FAST MODEXP 7^13 mod 11 -> RESULT: %s (EXPECTED: 2)\n", buf);

    printf("\n-3.2. NAIVE MODULAR EXPONENTIATION TESTING\n");
    BigInt *ans_naive = big_modexp_naive(b7, e13, m11);
    bigint_to_string(ans_naive, buf, sizeof(buf));
    printf("NAIVE MODEXP 7^13 mod 11 -> RESULT: %s (EXPECTED: 2)\n", buf);

    free_bigint(a);
    free_bigint(b);
    free_bigint(sum);
    free_bigint(diff);

    free_bigint(m1);
    free_bigint(m2);
    free_bigint(prod);

    free_bigint(b7);
    free_bigint(e13);
    free_bigint(m11);
    free_bigint(ans_fast);
    free_bigint(ans_naive);

    printf("=== HOANG'S UNIT TESTS COMPLETED ===\n");
}