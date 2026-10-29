#ifndef BIGINT_H
#define BIGINT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 10000 // 10^4
#define DIGIT_WIDTH 4

typedef struct
{
  uint32_t *digits; // little endian limb arr
  size_t size;      // cur num of act limbs
  size_t capacity;  // total allo capa
  int sign;         // 1 for pos, -1 for neg
} BigInt;

// core lifecycle, parsing
BigInt *create_bigint(size_t init_cap);
void free_bigint(BigInt *num);
void trim_leading_zeros(BigInt *num);
int compare_abs(const BigInt *a, const BigInt *b);

// string parse, format
BigInt *string_to_bigint(const char *str);
void bigint_to_string(const BigInt *num, char *buffer, size_t max_len);

// helpers
BigInt *int_to_bigint_internal(int64_t val);
BigInt *bigint_copy(const BigInt *src);
int is_zero(const BigInt *a);

// hoang
BigInt *bigint_add(const BigInt *a, const BigInt *b);
BigInt *bigint_sub(const BigInt *a, const BigInt *b);
BigInt *bigint_mul(const BigInt *a, const BigInt *b);
BigInt *big_modexp_naive(const BigInt *base, const BigInt *exp,
                         const BigInt *mod);
BigInt *big_modexp(const BigInt *base, const BigInt *exp, const BigInt *mod);

// compatibility aliases
#define bgint_add bigint_add
#define bgint_sub bigint_sub
#define bgint_mul bigint_mul
#define bgint_modexp_naive big_modexp_naive
#define bgint_modexp big_modexp

// phuc (division & modulo)
int big_divmod(const BigInt *u, const BigInt *v, BigInt **quotient, BigInt **remainder);
BigInt *big_div(const BigInt *u, const BigInt *v);
BigInt *big_mod(const BigInt *u, const BigInt *v);

// unit tests & benchmarks
void run_hoang_test(void);
void run_modexp_benchmark(void);
void run_arithmetic_oracle_tests(int num_iterations);
void run_phuc_tests(void);
void run_oracle_tests(void);
void run_division_benchmark(void);

#endif
