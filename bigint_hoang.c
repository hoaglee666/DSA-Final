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

// format bigint to stri buff
void bigint_to_string(const BigInt *num, char *buffer, size_t max_len)
{
    if (max_len == 0)
        return;
    size_t pos = 0;

    if (num->sign == -1 && (num->size == 1 && num->digits[0] == 0))
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
    BigInt *cur = bigint_mul(base, int_to_bigint_internal(1)); // copy base

    // extract scalar
    uint64_t e = 0;
    for (int i = (int)exp->size - 1; i >= 0; i--)
        e = e * BASE + exp->digits[i];

    for (uint64_t i = 0; i < e; i++)
    {
        BigInt *temp = bigint_mul(res, cur);
        // NOTE: NEED big_divmod REMAINDER (todo)
        free_bigint(res);
        res = temp;
    }
    free_bigint(cur);
    return res;
}

// square and mul
BigInt *big_modexp(const BigInt *base, const BigInt *exp, const BigInt *mod)
{
    BigInt *res = int_to_bigint_internal(1);
    BigInt *cur = bigint_mul(base, int_to_bigint_internal(1));

    uint64_t e = 0;
    for (int i = (int)exp->size - 1; i >= 0; i--)
        e = e * BASE + exp->digits[i];

    while (e > 0)
    {
        if (e & 1)
        {
            BigInt *temp = bigint_mul(res, cur);
            // TEMP = MOD RED VS MOD
            free_bigint(res);
            res = temp;
        }
        BigInt *squared_cur = bigint_mul(cur, cur);
        // squaredcur = mod red vs mod
        free_bigint(cur);
        cur = squared_cur;
        e >>= 1;
    }
    free_bigint(cur);
    return res;
}

void run_hoang_test()
{
    printf("=== RUNNING HOANG UNIT TESTS ===\n");

    BigInt *b7 = string_to_bigint("7");
    BigInt *e13 = string_to_bigint("13");
    BigInt *m11 = string_to_bigint("11");

    BigInt *ans = big_modexp(b7, e13, m11);

    char buf[128];
    bigint_to_string(ans, buf, sizeof(buf));
    printf("TEST 7^13 mod 11 -> RESULT: %s (EXPECTED BASED ON TRACE: 2)\n", buf);

    free_bigint(b7);
    free_bigint(e13);
    free_bigint(m11);
    free_bigint(ans);

    printf("=== UNIT TESTS COMPLETED ===\n");
}