# Week 2 Report — Implementation & Verification

**Project:** Big-int Division & Modular Exponentiation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Repository & Evidence Links:**

- **Public GitHub Repository:** [https://github.com/hoaglee666/DSA-Final.git]
- **Report File Link:** [https://github.com/hoaglee666/DSA-Final/blob/main/WEEKLY_REPORT_2.md]
- **Commit Link:** [https://github.com/hoaglee666/DSA-Final/commit/1319396972e5d1383b8a88bcade08c00a34afad4]
- **Source Files:**
  - Structure header: `bigint.h`
  - Hoàng's arithmetic & modexp: `bigint_hoang.c`
  - Phúc's division, oracle & benchmark: `bigint_phuc.c`
  - Runner: `main.c`

---

## 1. Workload [Shared]

| Student         | Work                                                                                                                                                                                                                                                                                              | Status   | Evidence                     |
| :-------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :------- | :--------------------------- |
| Đào Nguyên Phúc | Knuth's algorithm D (`big_divmod`), helpers (`big_div`, `big_mod`), signed division, unit tests, edge case suite, 1st complexity derivation draft                                                                                                                                                 | **Done** | `bigint_phuc.c`, `main.c`    |
| Lê Huy Hoàng    | Dynamic memory lifecycle (`create_bigint`, `free_bigint`), IO string conversion (`string_to_bigint`, `bigint_to_string`), basic arithmetic (`bigint_add`, `bigint_sub`, `bigint_mul`), fast modexp (`big_modexp`), naive modexp (`big_modexp_naive`), unit tests, 1st complexity derivation draft | **Done** | `bigint.h`, `bigint_hoang.c` |

## 2. Core structure

```c
typedef struct {
  uint32_t *digits; // little endian limb arr
  size_t size;      // cur num of act limbs
  size_t capacity;  // total allo capa
  int sign;         // 1 for pos, -1 for neg
} BigInt;
```

## 3. Unit tests [Shared]

Log from `./final`

```plaintext
=====================================================
=== DSA FINAL PROJECT: BIGINT TEST & BENCHMARK SUITE
=====================================================

=== RUNNING HOANG UNIT TESTS ===

1. ADDITION & SUBTRACTION TESTING
987654321 + 123456789 = 1111111110 (EXPECTED 1111111110)
987654321 - 123456789 = 864197532 (EXPECTED 864197532)

2. MULTIPLICATION TESTING
1234 * 5678 = 7006652 (EXPECTED 7006652)

3. ModExp TESTING

-3.1. FAST MODULAR EXPONENTIATION TESTING
[FAST MODEXP] EXPONENT 13 IN BINARY is 1101_2 (4 bits)
INITIAL STATE: r = 1, base b = 7
-> STEP 1 (BIT = 1, MSB): SQUARE r^2 mod m = 1; MULTIPLY BY BASE -> 7 mod m
UPDATED ACCUMULATOR -> r = 7
-> STEP 2 (BIT = 1): SQUARE r^2 mod m = 5; MULTIPLY BY BASE -> 2 mod m
UPDATED ACCUMULATOR -> r = 2
-> STEP 3 (BIT = 0): SQUARE r^2 mod m = 4 (MULTIPLY SKIPPED)
UPDATED ACCUMULATOR -> r = 4
-> STEP 4 (BIT = 1): SQUARE r^2 mod m = 5; MULTIPLY BY BASE -> 2 mod m
UPDATED ACCUMULATOR -> r = 2
FAST MODEXP 7^13 mod 11 -> RESULT: 2 (EXPECTED: 2)

-3.2. NAIVE MODULAR EXPONENTIATION TESTING
[NAIVE TRACE START] BASE b = 7, EXPONENT e = 13, INITIAL ACCUMULATOR r = 1
-> NAIVE STEP 1/13: r * b = 7 -> MOD REDUCTION -> NEW r = 7
-> NAIVE STEP 2/13: r * b = 49 -> MOD REDUCTION -> NEW r = 5
-> NAIVE STEP 3/13: r * b = 35 -> MOD REDUCTION -> NEW r = 2
-> NAIVE STEP 4/13: r * b = 14 -> MOD REDUCTION -> NEW r = 3
-> NAIVE STEP 5/13: r * b = 21 -> MOD REDUCTION -> NEW r = 10
-> NAIVE STEP 6/13: r * b = 70 -> MOD REDUCTION -> NEW r = 4
-> NAIVE STEP 7/13: r * b = 28 -> MOD REDUCTION -> NEW r = 6
-> NAIVE STEP 8/13: r * b = 42 -> MOD REDUCTION -> NEW r = 9
-> NAIVE STEP 9/13: r * b = 63 -> MOD REDUCTION -> NEW r = 8
-> NAIVE STEP 10/13: r * b = 56 -> MOD REDUCTION -> NEW r = 1
-> NAIVE STEP 11/13: r * b = 7 -> MOD REDUCTION -> NEW r = 7
-> NAIVE STEP 12/13: r * b = 49 -> MOD REDUCTION -> NEW r = 5
-> NAIVE STEP 13/13: r * b = 35 -> MOD REDUCTION -> NEW r = 2
NAIVE MODEXP 7^13 mod 11 -> RESULT: 2 (EXPECTED: 2)
=== HOANG'S UNIT TESTS COMPLETED ===

=== RUNNING PHUC'S WEEK 2 UNIT TESTS ===
[Test 1] Week 1 Division Trace: 95432 / 187
  Result:   q = 510, r = 62 (status=0)
  Expected: q = 510, r = 62
  -> PASS!
[Test 2] Edge Case: Division by zero
  -> PASS (Correctly rejected division by zero with error code -1)
[Test 3] Edge Case: u < v (187 / 95432)
  -> PASS (q = 0, r = 187)
[Test 4] Edge Case: u == v (95432 / 95432)
  -> PASS (q = 1, r = 0)
[Test 5] Edge Case: Divisor is 1 (95432 / 1)
  -> PASS (q = 95432, r = 0)
[Test 6] Multi-limb Knuth Algorithm D: 123456789012345 / 12345678
  Result:   q = 10000000, r = 9012345
  Expected: q = 10000000, r = 9012345
  -> PASS!
[Test 7] Step D6 Add-Back Activation: 1000000000000 / 500000009999
  Result:   q = 1, r = 499999990001
  Expected: q = 1, r = 499999990001
  -> PASS (Add-back executed correctly!)
[Test 8] Signed Division Semantics:
  (-95432) / 187: q = -510, r = -62
  -> PASS!
=== PHUC'S UNIT TESTS COMPLETED ===

=== RUNNING AUTOMATED ORACLE TESTS ===
[Oracle Suite 1] Testing against native 64-bit hardware integer division (500 cases)...
  -> Oracle 64-bit passed: 500 / 500
[Oracle Suite 2] Testing mathematical invariant (u = q * v + r, 0 <= r < v) on 200 large BigInts...
  -> Large Invariant passed: 200 / 200
=== ALL ORACLE TESTS COMPLETED: 700 / 700 PASSED (100.0%) ===

=== EMPIRICAL COMPLEXITY ANALYSIS FOR KNUTH ALGORITHM D ===
Measuring division runtime across scaling limb sizes:
Limbs (N)  | Digits (~4N) | Iterations      | Avg Time (us)   | T(N)/T(N/2)
----------------------------------------------------------------------
10         | 40           | 500             | 0.94            | -
20         | 80           | 500             | 3.05            | 3.25x
40         | 160          | 500             | 10.83           | 3.55x
80         | 320          | 100             | 55.25           | 5.10x
160        | 640          | 100             | 239.05          | 4.33x
320        | 1280         | 20              | 913.65          | 3.82x
640        | 2560         | 5               | 3634.00         | 3.98x
----------------------------------------------------------------------
Theoretical Knuth Algorithm D complexity: O(m * n).
When doubling both m and n (N -> 2N), expected theoretical growth is 4.00x (O(N^2)).
=== BENCHMARK COMPLETED ===

All test suites and benchmarks completed successfully.
```

## 4. First complexity derivation [Shared]

A fixed limb base, limbs are stored in an array, least significant limb first.

- L be number of limbs used in modulus and intermediate values in modexp.
- e be exponent value converted to `uint64_t`.
- u be dividend, v divisor in `big_divmod`.
- `n = v->size`, `m = u->size - n` for multi limb division where u > v.

### 4.1. Fast modular exponentiation (`big_modexp`) [Hoàng]

`big_modexp` uses repeated squaring: square the result of exponent bit and multiply by base if bit is 1.

Each multiplication takes $O(L^2)$ time for numbers with L limbs. With modulus, results are reduced using `big_mod`
which call `big_divmod` and take $O(L^2)$ for same sized operands.

Exponent with k bits and operands of at most L limbs, time complexity is $O(kL^2)$. Since exponent is converted to `uint64_t`, it only handles up to 64 exponent bits.

### 4.2. Long division [Phúc]

`big_divmod` uses long division for multi limb divisor. If dividend has $m + n$ limbs, divisor $n$ limbs, it processes roughly $m + 1$ quotient positions. Each position performs same to $n$, to esitimate quotient digit and multi/subtract divisor.

Time complexity is $O((m + 1)n)$, $O(L^2) when operands have same sizes$. Single limb divisor takes $O(u)$ time, u is number of dividend limbs.

## 5. AI log & error [Shared]

| Phase  | Student | Prompt                                             | Output                                                                                               | error                                                                                                                                                               | Correction                                             |
| :----- | :------ | :------------------------------------------------- | :--------------------------------------------------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :----------------------------------------------------- |
| Week 2 | Hoàng   | improve my fast modexp                             | `res = mod_squared_res;` inside `if-else`, call `free_bigint(mod_squared_res)` unconditionally later | Memory double free, caused segmentation fault on scanning bit because active accumulator freed                                                                      | Free `mod_squared_res` when bit = 1                    |
| Week 2 | Phúc    | draft algorithm d normalization and trial quotient | Omitted normalization step d = 1                                                                     | Algorithm correctness, without $v'_{n-1} \geq \lfloor BASE/2 \rfloor$, trial quotient can overshoot by more than 2, violate Knuth's invariant, make remainder wrong | Add normalization step D1, add step D8 unnormalization |

## 6. Teach back [Shared]

### 6.1. `big_modexp` [Hoàng]

**Line level explanation (trimmed)**:

```c
BigInt *big_modexp(const BigInt *base, const BigInt *exp, const BigInt *mod)
{
    BigInt *res = int_to_bigint_internal(1); //initialize result accumlator r to 1
    BigInt *cur = bigint_copy(base); //copy of base, original value not modified
    if (mod)
    {
        BigInt *reduced = big_mod(cur, mod);
        free_bigint(cur);
        cur = reduced;
    } //if have modulus, reduce base modulo mod before calculate.

    uint64_t e = 0; //create 64bit integer to store exp
    for (int i = (int)exp->size - 1; i >= 0; i--)
        e = e * BASE + exp->digits[i]; //convert exp from bigint to standard int.

    //...

    // most significant bit-least significant bit, set msb to 1
    int highest_bit = -1;
    for (int b_idx = 63; b_idx >= 0; b_idx--)
    {
        if ((e >> b_idx) & 1)
        {
            highest_bit = b_idx;
            break;
        }
    }

    //...

    for (int b_idx = highest_bit; b_idx >= 0; b_idx--) //process exp bit one at a time, msb-lsb
    {
        int bit = (e >> b_idx) & 1; //extract current exp bit, 1 or 0

        // square opera: r^2 mod m
        BigInt *squared_res = bigint_mul(res, res);
        BigInt *mod_squared_res = mod ? big_mod(squared_res, mod) : bigint_copy(squared_res);
        free_bigint(squared_res); //reduce squared result if modulus is provided

        if (bit) //check if bit = 1
        {
            // snm by base
            BigInt *mult = bigint_mul(mod_squared_res, cur); //multi by base
            BigInt *mod_mult = mod ? big_mod(mult, mod) : bigint_copy(mult); //reduce result

            res = mod_mult; //update accum
        }
        else
        {
            // bit = 0
            res = mod_squared_res; //update accum with squared result, skip multi
        }
        //...
    }

    free_bigint(cur);
    return res; //final result
}
```

How it works, for each exponent bit:

1. Square current result: $r \leftarrow r^2$.
2. Reduce modulo $mod$.
3. If current bit is 1, multi by base: $r \leftarrow r \times base$, skip if 0.
4. Reduce module $mod$ again, if possible.

**Loop invariant:**

1. At start of iteration, $res$ is base raised to power of exponent bits already processed. If modulus provided, $res$ is that value modulo $mod$. If processed bits have numberical value $p$: $res \equiv base^p \pmod {mod}$
2. First, $res = 1$, before processing bits, $p = 0$, $base^0 = 1$, invariant true.
3. For next bit b, processed exponent become $2p + b$. First, square $res$, $base^{2p}$. If b = 1, multiply by base to get $base^{2p + 1}$, b = 0; skip multiplication. Invariant true.
4. After exponent bits processed, $p = exp$. Function returns $res \equiv base^{exp} \pmod {mod}$

**Test output**:

```plaintext
-3.1. FAST MODULAR EXPONENTIATION TESTING
[FAST MODEXP] EXPONENT 13 IN BINARY is 1101_2 (4 bits)
INITIAL STATE: r = 1, base b = 7
-> STEP 1 (BIT = 1, MSB): SQUARE r^2 mod m = 1; MULTIPLY BY BASE -> 7 mod m
UPDATED ACCUMULATOR -> r = 7
-> STEP 2 (BIT = 1): SQUARE r^2 mod m = 5; MULTIPLY BY BASE -> 2 mod m
UPDATED ACCUMULATOR -> r = 2
-> STEP 3 (BIT = 0): SQUARE r^2 mod m = 4 (MULTIPLY SKIPPED)
UPDATED ACCUMULATOR -> r = 4
-> STEP 4 (BIT = 1): SQUARE r^2 mod m = 5; MULTIPLY BY BASE -> 2 mod m
UPDATED ACCUMULATOR -> r = 2
FAST MODEXP 7^13 mod 11 -> RESULT: 2 (EXPECTED: 2)
```

### 6.2. `big_divmod` [Phúc]

**Line level explanation (trimmed)**:

```c
// Line 1-12: Normalization Step D1
uint32_t vn_1 = v->digits[n - 1];
uint32_t d = (uint32_t)(BASE / (vn_1 + 1)); // Normalization factor d
// Multiply dividend u by d into u_prime (size m + n + 1)
// Multiply divisor v by d into v_prime (size n)

// Line 13-15: Outer Loop Step D2 (j from m down to 0)
for (int j = (int)m; j >= 0; j--) {
    // Line 16-24: Step D3 (Quotient Estimate q_hat)
    uint64_t num = (uint64_t)u_prime[j + n] * BASE + u_prime[j + n - 1];
    uint64_t q_hat = num / v_prime[n - 1];
    uint64_t r_hat = num % v_prime[n - 1];

    // Refine q_hat using 2-limb test condition: q_hat * v'_{n-2} > BASE * r_hat + u'_{j+n-2}
    while (r_hat < BASE && (q_hat * v_prime[n - 2] > r_hat * BASE + u_prime[j + n - 2])) {
        q_hat--;
        r_hat += v_prime[n - 1];
    }

    // Line 25-34: Step D4 (Multiply q_hat * v_prime and Subtract from u_prime window)
    uint64_t mul_carry = 0;
    for (size_t i = 0; i < n; i++) {
        uint64_t prod = (uint64_t)q_hat * v_prime[i] + mul_carry;
        p_scratch[i] = (uint32_t)(prod % BASE);
        mul_carry = prod / BASE;
    }
    // Subtract p_scratch from u_prime[j .. j+n] with borrow handling

    // Line 35-42: Step D5 & D6 (Test Borrow & Step D6 Add-Back Correction)
    if (sub_borrow != 0) {
        q_hat--; // q_hat overshot by 1
        // Add v_prime back to u_prime[j .. j+n]
    }
    q->digits[j] = (uint32_t)q_hat; // Store quotient digit
}

// Line 43-48: Step D8 (Unnormalize remainder by dividing u_prime by d)
```

**Loop invariant:**

Main loop processes quotient digits from highest position $m$ to 0.

1. At start of iteration, higher quotients already calculated. Remaining portion of normalized dividend is less than $v' \cdot BASE$, so next quotient digit represented by single limb.
2. At $j = m$, no quotient digits calculated. Since normalized dividend has at most $m + n + 1$ limbs, first quotient digit estimated from its highest limbs.
3. Algorithm estimate next quotient digit $\hat{q}$, correct if necessary, subtract $\hat{q} \cdot v'$ from divident portion. If estimate too large, addback corrects it. Result remainder smaller than $v'$, so next lower quotient digit to be calculated.
4. When loop finishes, all quotient digits determined. Remaining low $n$ limbs represent normalized remainder, divided by $d$ to recover original remainder. Function returns quotient and remainder.

**Test output**:

```plaintext
[Test 1] Week 1 Division Trace: 95432 / 187
  Result:   q = 510, r = 62 (status=0)
  Expected: q = 510, r = 62
  -> PASS!
[Test 2] Edge Case: Division by zero
  -> PASS (Correctly rejected division by zero with error code -1)
[Test 3] Edge Case: u < v (187 / 95432)
  -> PASS (q = 0, r = 187)
[Test 4] Edge Case: u == v (95432 / 95432)
  -> PASS (q = 1, r = 0)
[Test 5] Edge Case: Divisor is 1 (95432 / 1)
  -> PASS (q = 95432, r = 0)
[Test 6] Multi-limb Knuth Algorithm D: 123456789012345 / 12345678
  Result:   q = 10000000, r = 9012345
  Expected: q = 10000000, r = 9012345
  -> PASS!
[Test 7] Step D6 Add-Back Activation: 1000000000000 / 500000009999
  Result:   q = 1, r = 499999990001
  Expected: q = 1, r = 499999990001
  -> PASS (Add-back executed correctly!)
[Test 8] Signed Division Semantics:
  (-95432) / 187: q = -510, r = -62
  -> PASS!
```
