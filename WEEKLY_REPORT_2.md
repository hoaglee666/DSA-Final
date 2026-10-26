# Week 2 Report — Implementation & Verification

**Project:** Big-int Division & Modular Exponentiation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Repository & Evidence Links:**
- **Public GitHub Repository:** [https://github.com/hoaglee666/DSA-Final](https://github.com/hoaglee666/DSA-Final)
- **Week 2 Target Commit:** [`1319396972`](https://github.com/hoaglee666/DSA-Final/commit/1319396972e5d1383b8a88bcade08c00a34afad4) (Oct 25, 2026)
- **Week 1 Report:** [WEEKLY_REPORT_1.md](WEEKLY_REPORT_1.md)
- **Source Files (direct links):**
  - [`bigint.h`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint.h) — BigInt struct definition and function prototypes
  - [`bigint_hoang.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_hoang.c) — Memory lifecycle, add/sub/mul, naive & fast modexp
  - [`bigint_phuc.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_phuc.c) — Knuth Algorithm D, oracle test suite, benchmarks
  - [`main.c`](https://github.com/hoaglee666/DSA-Final/blob/main/main.c) — Test and benchmark runner

---

## 1. Workload [Shared]

The tasks completed in Week 2 match the plan we laid out in Week 1:

| Member | Tasks Completed | Status | Evidence |
|:-------|:----------------|:-------|:---------|
| **Đào Nguyên Phúc** | Implemented Knuth's Algorithm D (`big_divmod`), helper functions (`big_div`, `big_mod`), signed division, 8 unit tests with edge cases, 700-case automated oracle test suite, and division empirical complexity benchmark. | **Done** | [`bigint_phuc.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_phuc.c), `main.c` |
| **Lê Huy Hoàng** | Implemented BigInt memory lifecycle (`create_bigint`, `free_bigint`), string conversion (`string_to_bigint`, `bigint_to_string`), basic arithmetic (`bigint_add`, `bigint_sub`, `bigint_mul`), naive modexp (`big_modexp_naive`), fast square-and-multiply modexp (`big_modexp`), and modexp benchmarks. | **Done** | [`bigint_hoang.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_hoang.c), `bigint.h` |

---

## 2. Technical Walkthrough: Knuth's Algorithm D [Phúc]

The division routine is declared in [`bigint.h`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint.h):
```c
int big_divmod(const BigInt *u, const BigInt *v, BigInt **quotient, BigInt **remainder);
```

### 2.1. Fast Paths & Edge Cases
Before running multi-limb division, we handle simple cases directly:
- **Divisor is zero ($v = 0$):** Return `-1` and set both output pointers to `NULL`.
- **$|u| < |v|$:** Quotient is `0`, remainder is a copy of `u`.
- **$|u| == |v|$:** Quotient is `sign(u) * sign(v)`, remainder is `0`.
- **Single-limb divisor ($n = 1$):** Run simple short division with one loop ($O(m)$), avoiding the overhead of normalization arrays.

### 2.2. Multi-Limb Steps (Algorithm D, $n \ge 2$)
1. **D1 (Normalize):**
   Calculate scale factor $d = \lfloor \text{BASE} / (v_{n-1} + 1) \rfloor$, where $\text{BASE} = 10000$. Multiply $u$ by $d$ into array $u'$ (size $m + n + 1$) and $v$ by $d$ into array $v'$ (size $n$). This ensures $v'_{n-1} \ge \lfloor \text{BASE} / 2 \rfloor = 5000$.
2. **D2 (Loop $j$):**
   Set loop index $j$ running from $m$ down to $0$, where $m = \text{size}(u) - \text{size}(v)$.
3. **D3 (Estimate $\hat{q}$):**
   Compute trial quotient $\hat{q} = \lfloor (u'_{j+n} \cdot \text{BASE} + u'_{j+n-1}) / v'_{n-1} \rfloor$ and remainder $\hat{r}$. If $u'_{j+n} == v'_{n-1}$, set $\hat{q} = \text{BASE} - 1$.
   Refine $\hat{q}$ with Knuth's 2-limb condition:
   $$\hat{q} \cdot v'_{n-2} > \text{BASE} \cdot \hat{r} + u'_{j+n-2}$$
   Decrement $\hat{q}$ by 1 and increase $\hat{r}$ by $v'_{n-1}$ while this holds (Knuth showed this loop executes at most twice).
4. **D4 (Multiply and Subtract):**
   Multiply $P = \hat{q} \times v'$ and subtract it from the window $u'[j \dots j + n]$.
5. **D5 & D6 (Test Borrow and Add-Back):**
   If the subtraction produced a borrow (meaning $\hat{q}$ was 1 too large), decrement $\hat{q}$ by 1 and add $v'$ back to $u'[j \dots j + n]$.
6. **D7 (Store Digit):**
   Set quotient limb $q_j = \hat{q}$.
7. **D8 (Unnormalize):**
   Divide $u'[0 \dots n - 1]$ by $d$ using short division to get the true remainder. The division is exact with no scalar remainder.

---

## 3. Unit Test Results [Shared]

Console output from running `./final.exe`:

```text
=== RUNNING HOANG UNIT TESTS ===
TEST 7^13 mod 11 -> RESULT: 2 (EXPECTED BASED ON TRACE: 2)
=== UNIT TESTS COMPLETED ===

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
```

### Trace Checks
- **Division trace from Week 1 (Section 4.3):**
  $95432 \div 187 = 510$ remainder $62$.
  Check: $510 \times 187 + 62 = 95370 + 62 = 95432$ ✓.
- **Modexp trace from Week 1 (Section 4.2):**
  $7^{13} \pmod{11} = 2$. Output matches 2 ✓.

---

## 4. Automated Oracle Testing [Phúc]

To make sure division works on arbitrary numbers and not just our hand-crafted tests, we ran two automated oracle test suites:

```text
=== RUNNING AUTOMATED ORACLE TESTS ===
[Oracle Suite 1] Testing against native 64-bit hardware integer division (500 cases)...
  -> Oracle 64-bit passed: 500 / 500
[Oracle Suite 2] Testing mathematical invariant (u = q * v + r, 0 <= r < v) on 200 large BigInts...
  -> Large Invariant passed: 200 / 200
=== ALL ORACLE TESTS COMPLETED: 700 / 700 PASSED (100.0%) ===
```

1. **Hardware Oracle (500 tests):**
   Generates random 64-bit numbers and compares `big_divmod` against C native `/` and `%`. All 500 cases matched.
2. **Mathematical Invariant Oracle (200 tests):**
   Generates random numbers between 20 and 100 decimal digits (larger than 64-bit integers) and checks two rules:
   - $0 \le r < |v|$
   - $q \times v + r = u$ (reconstructed with `bigint_mul` and `bigint_add`).
   All 200 cases passed.

---

## 5. Empirical Complexity & Benchmarks [Shared]

### 5.1. Division Benchmark (`big_divmod`) [Phúc]

We tested division timing by setting dividend size to $2N$ limbs and divisor size to $N$ limbs, scaling $N$ from 10 to 640 limbs (up to ~2560 decimal digits). Compiled with GCC `-O2` on Windows:

| Limbs ($N$) | Digits ($\approx 4N$) | Iterations | Avg Time ($\mu\text{s}$) | Ratio $T(N) / T(N/2)$ |
|:---:|:---:|:---:|:---:|:---:|
| 10  | 40   | 500 | $< 1.0\,\mu\text{s}$ | — |
| 20  | 80   | 500 | $2.00\,\mu\text{s}$ | — |
| 40  | 160  | 500 | $4.00\,\mu\text{s}$ | $2.00\times$ |
| 80  | 320  | 100 | $10.00\,\mu\text{s}$ | $2.50\times$ |
| 160 | 640  | 100 | $50.00\,\mu\text{s}$ | $5.00\times$ |
| 320 | 1280 | 20  | $250.00\,\mu\text{s}$ | $5.00\times$ |
| 640 | 2560 | 5   | $1000.00\,\mu\text{s}$ | $4.00\times$ |

**Analysis:**
- In Algorithm D, there are $(m + 1)$ outer iterations, and each iteration does an $O(n)$ multiply-and-subtract. So total time is $O((m + 1) \cdot n)$. When $m = n = N$, this is $O(N^2)$.
- Doubling $N \rightarrow 2N$ gives theoretical ratio $(2N)^2 / N^2 = 4.0\times$. Our measured ratios converge to $4.0\times$ as numbers get large enough to overcome timer granularity.

### 5.2. Modular Exponentiation Benchmark (`big_modexp`) [Hoàng]

As requested in the Week 1 comment ("keep benchmark modexp 16/64/256/1024 bit with specific measurements"), we benchmarked `big_modexp` using square-and-multiply with operands across 16, 64, 256, and 1024 bits. The exponent was chosen equal in size to the modulus:

| Bit size | Base/Mod limbs | Digits | Iterations | Avg Time ($\mu\text{s}$) | Scaling Factor |
|:---:|:---:|:---:|:---:|:---:|:---:|
| 16   | 2 limbs | ~5   | 1000 | $0.50\,\mu\text{s}$ | — |
| 64   | 5 limbs | ~20  | 500  | $8.00\,\mu\text{s}$ | $16.0\times$ |
| 256  | 20 limbs | ~78  | 100  | $350.00\,\mu\text{s}$ | $43.8\times$ |
| 1024 | 78 limbs | ~309 | 10   | $22000.00\,\mu\text{s}$ (~22 ms) | $62.9\times$ |

**Analysis:**
- Fast modexp uses $O(\log e)$ squarings and multiplications. Each multiplication takes $O(n^2)$ limb operations, followed by a modulo reduction (`big_mod`) which also takes $O(n^2)$. Total time is $O(n^2 \log e)$.
- When bit width quadruples (16 $\rightarrow$ 64 $\rightarrow$ 256 $\rightarrow$ 1024), limb count $n$ roughly quadruples, making each multiplication $\approx 16\times$ slower, plus $\approx 4\times$ more exponent bits to scan.
- Why square-and-multiply is necessary: at 1024 bits, fast modexp takes 22 ms. Naive modexp would need $2^{1024}$ multiplications, which would take longer than the age of the universe.

---

## 6. AI Log & Real Bugs Encountered [Shared]

We used GitHub Copilot for syntax suggestions and boilerplate (like `snprintf` format specifiers and test harness loops). We wrote all algorithm logic ourselves. During development, AI suggestions caused two real bugs:

| Phase | Member | Prompt / Suggestion | AI Output | Bug Encountered | How We Fixed It |
|:------|:-------|:-------------------|:----------|:----------------|:----------------|
| Week 2 | Hoàng | Cleanup loop for `big_modexp_naive` | `free_bigint(temp); res = big_mod(temp, mod);` | **Double Free / Use-After-Free**: Calling `big_mod` on `temp` after freeing it caused a segmentation fault. | Reordered operations: call `new_res = big_mod(temp, mod)` first, then `free_bigint(temp)` and `free_bigint(res)`, then assign `res = new_res`. |
| Week 2 | Phúc | Initial scaffold for Knuth Algorithm D | Skipped normalization, calculated $\hat{q}$ directly from raw limbs $u_{j+n} / v_{n-1}$ | **Algorithm Correctness**: Without $v'_{n-1} \ge \lfloor \text{BASE} / 2 \rfloor$, trial quotient can overshoot by 2 or more, breaking Knuth's invariant. Test 6 failed. | Added Step D1 (multiply $u$ and $v$ by $d = \lfloor \text{BASE} / (v_{n-1} + 1) \rfloor$) and Step D8 (divide remainder by $d$). |

---

## 7. Week 3 Plan [Shared]

Here is our division of work for Week 3 (4-week course):

### 7.1. Đào Nguyên Phúc (22110062)
- **Detailed Modexp Timing Logs (2 hours):** Save detailed log files showing exact cycle counts and timestamps for 16/64/256/1024-bit test runs to satisfy the specific measurement requirement.
- **Algorithm D Performance Tuning (3 hours):** Profile limb allocation in `big_divmod` to reuse temporary scratchpad memory across loop iterations instead of reallocating.
- **Report Writing (2 hours):** Draft the Week 3 technical analysis, benchmark charts, and conclusions.

### 7.2. Lê Huy Hoàng (22110026)
- **Fast vs. Naive Modexp Comparison (3 hours):** Run `big_modexp` and `big_modexp_naive` side-by-side up to the point where naive times out, plotting empirical speedup ratios.
- **Arithmetic Oracle Testing (2 hours):** Write randomized tests comparing `bigint_mul` against native 64-bit and 128-bit integers (`__int128`).
- **Valgrind Audit & Demo Prep (2 hours):** Run Valgrind across the full test suite to guarantee zero leaks, and prepare code walkthrough notes for the oral defense.

---

## 8. Appendix / Question on Rubric [Shared]

Regarding our question submitted earlier about Appendix B (question family NUMERIC) in the project rubric: we noted the instructor's feedback that this will be answered during the final oral defense / viva session. We will prepare our answers around multi-precision representation and complexity trade-offs accordingly.

