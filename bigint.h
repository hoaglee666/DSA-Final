#ifndef BIGINT_H
#define BIGINT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 10000 // 10^4
#define DIGIT_WIDTH 4

typedef struct {
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

// hoang
BigInt *bgint_add(const BigInt *a, const BigInt *b);
BigInt *bgint_sub(const BigInt *a, const BigInt *b);
BigInt *bgint_mul(const BigInt *a, const BigInt *b);
BigInt *bgint_modexp_naive(const BigInt *base, const BigInt *exp,
                           const BigInt *mod);
BigInt *bgint_modexp(const BigInt *base, const BigInt *exp, const BigInt *mod);

// unit test
void run_hoang_test();

#endif
