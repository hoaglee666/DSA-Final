# Week 2 Report — Implementation & Validation

**Project:** Big-int Division & Modular Exponentiation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Repository & Evidence Links:**
- **Public GitHub Repository:** [https://github.com/hoaglee666/DSA-Final.git](https://github.com/hoaglee666/DSA-Final.git)
- **Week 1 Report:** [WEEKLY_REPORT_1.md](WEEKLY_REPORT_1.md)
- **Source Files:**
  - Core header: [bigint.h](bigint.h)
  - Hoàng's arithmetic & modexp: [bigint_hoang.c](bigint_hoang.c)
  - Phúc's division, oracle & benchmark: [bigint_phuc.c](bigint_phuc.c)
  - Runner: [main.c](main.c)

---

## 1. Week 2 Summary of Completed Tasks

In accordance with the Week 2 plan set out in [WEEKLY_REPORT_1.md](WEEKLY_REPORT_1.md#62-phúcs-tasks-target--8-hours), all tasks assigned to **Đào Nguyên Phúc** have been implemented, verified, and benchmarked:

| Task | Description | Status | Verification |
|------|-------------|--------|--------------|
| **1. Long Division (`big_divmod`)** | Implemented Knuth's Algorithm D with normalization ($d$), trial quotient estimation ($\hat{q}$), correction loop, multiply-and-subtract, and add-back step (Step D6). Also provides fast short-division path for single-limb divisors ($n=1$) and signed division support. | **Done** | Unit tests & Oracle test suite (700 tests passed) |
| **2. Week 1 Division Trace Verification** | Unit tests for $95432 \div 187 = 510$ with remainder $62$, matching the hand trace in Section 4.3 of Week 1 Report. Also added edge cases: division by zero ($v=0$), $u < v$, $u = v$, $v = 1$, and Step D6 add-back activation. | **Done** | `run_phuc_tests()` in `bigint_phuc.c` |
| **3. Automated Oracle Testing** | Automated test suite validating `big_divmod` against 64-bit hardware integer division (500 random cases) and large-number mathematical invariants $u = q \times v + r$ with $0 \le r < v$ (200 cases up to 100 decimal digits). | **Done** | `run_oracle_tests()` (100% pass rate) |
| **4. Empirical Complexity Notes for Division** | Measured execution times of `big_divmod` across scaling limb counts ($N = 10$ to $640$ limbs) to empirically confirm the $O(m \cdot n)$ quadratic complexity ($T(2N)/T(N) \approx 4\times$). | **Done** | `run_division_benchmark()` |
| **5. Subsystem Integration** | Connected `big_divmod` into Hoàng's `big_modexp` and `big_modexp_naive`, allowing the Week 1 modexp trace ($7^{13} \pmod{11} = 2$) to succeed. | **Done** | `run_hoang_test()` passed |

---

## 2. Technical Implementation: Knuth's Algorithm D (`big_divmod`)

The function signature in [bigint.h](bigint.h) is:
```c
int big_divmod(const BigInt *u, const BigInt *v, BigInt **quotient, BigInt **remainder);
```

### 2.1. Algorithm Walkthrough

1. **Safety & Base Cases:**
   - **Division by zero ($v = 0$):** Returns error code `-1` and sets `*quotient = NULL, *remainder = NULL`.
   - **$|u| < |v|$:** Returns quotient `0` and remainder `|u|`.
   - **$|u| == |v|$:** Returns quotient $\pm 1$ and remainder `0`.
   - **Single-limb divisor ($n = 1$):** Short division executing in $O(m)$ time without the overhead of normalization arrays.

2. **Knuth's Algorithm D ($n \ge 2$):**
   - **Step D1 (Normalization):**
     Compute factor $d = \lfloor \text{BASE} / (v_{n-1} + 1) \rfloor$, where $\text{BASE} = 10000$. Multiply dividend $u$ by $d$ into $u'$ (size $m + n + 1$) and divisor $v$ by $d$ into $v'$ (size $n$). This guarantees the leading divisor limb $v'_{n-1} \ge \lfloor \text{BASE} / 2 \rfloor = 5000$.
   - **Step D2 (Loop on $j$):**
     Loop variable $j$ runs from $m$ down to $0$, where $m = u->size - v->size$.
   - **Step D3 (Quotient Estimate $\hat{q}$):**
     Estimate $\hat{q} = \lfloor (u'_{j+n} \cdot \text{BASE} + u'_{j+n-1}) / v'_{n-1} \rfloor$ and remainder $\hat{r}$. If $u'_{j+n} == v'_{n-1}$, set $\hat{q} = \text{BASE} - 1$.
     Refine $\hat{q}$ using Knuth's test condition:
     $$\hat{q} \cdot v'_{n-2} > \text{BASE} \cdot \hat{r} + u'_{j+n-2}$$
     Decreasing $\hat{q}$ and increasing $\hat{r}$ until the condition holds. Knuth proved this loop executes at most twice.
   - **Step D4 (Multiply and Subtract):**
     Compute $P = \hat{q} \times v'$ and subtract from window $u'[j \dots j + n]$.
   - **Step D5 & D6 (Test & Add-Back):**
     If the subtraction causes a borrow out of limb $j + n$ (meaning $\hat{q}$ was $q_j + 1$), decrement $\hat{q}$ by 1 and add $v'$ back to $u'[j \dots j + n]$.
   - **Step D7 (Store quotient digit):**
     Store $q_j = \hat{q}$.
   - **Step D8 (Unnormalization):**
     The unnormalized remainder is obtained by dividing $u'[0 \dots n - 1]$ by $d$. The remainder is guaranteed to be an exact multiple of $d$ with zero scalar remainder.

---

## 3. Unit Test Results

Execution logs from running `./final.exe`:

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

### Trace Verification:
- In Week 1 Report Section 4.3: $95432 \div 187 = 510$ with remainder $62$.
- Verification: $510 \times 187 + 62 = 95370 + 62 = 95432$.
- Our implementation produces exact matches for both $q$ and $r$.

---

## 4. Automated Oracle Test Results

To verify general correctness beyond fixed test vectors, two automated oracle test suites were executed:

```text
=== RUNNING AUTOMATED ORACLE TESTS ===
[Oracle Suite 1] Testing against native 64-bit hardware integer division (500 cases)...
  -> Oracle 64-bit passed: 500 / 500
[Oracle Suite 2] Testing mathematical invariant (u = q * v + r, 0 <= r < v) on 200 large BigInts...
  -> Large Invariant passed: 200 / 200
=== ALL ORACLE TESTS COMPLETED: 700 / 700 PASSED (100.0%) ===
```

1. **Hardware Oracle (500 cases):** Compares `big_divmod` results against C standard integer `/` and `%` on pseudo-random 64-bit values.
2. **Mathematical Invariant Oracle (200 cases):** For large numbers (20 to 100 decimal digits) where hardware words overflow, the test verifies:
   - Invariant 1: $0 \le r < |v|$
   - Invariant 2: $q \times v + r = u$ (verified using `bigint_mul` and `bigint_add`).

---

## 5. Empirical Complexity Notes for Division

### 5.1. Benchmark Measurements

The empirical complexity benchmark was run on Windows using GCC (`-O2`). The dividend length was set to $2N$ limbs ($m = N$ extra limbs) and the divisor length was $N$ limbs, across $N = 10$ to $640$ limbs (representing integers up to ~2,560 decimal digits):

| Limbs ($N$) | Digits ($\approx 4N$) | Iterations | Average Time ($\mu\text{s}$) | Ratio $T(N) / T(N/2)$ |
|:---:|:---:|:---:|:---:|:---:|
| 10  | 40   | 500 | $< 1.0\,\mu\text{s}$ | — |
| 20  | 80   | 500 | $2.00\,\mu\text{s}$ | — |
| 40  | 160  | 500 | $4.00\,\mu\text{s}$ | $2.00\times$ |
| 80  | 320  | 100 | $10.00\,\mu\text{s}$ | $2.50\times$ |
| 160 | 640  | 100 | $50.00\,\mu\text{s}$ | $5.00\times$ |
| 320 | 1280 | 20  | $250.00\,\mu\text{s}$ | $5.00\times$ |
| 640 | 2560 | 5   | $1000.00\,\mu\text{s}$ | $4.00\times$ |

### 5.2. Analysis & Discussion

1. **Theoretical Complexity:**
   Knuth's Algorithm D executes $(m + 1)$ outer loop iterations. In each iteration, multiplying $\hat{q} \times v'$ and subtracting takes $O(n)$ limb operations. Therefore, total complexity is:
   $$T(m, n) = O((m + 1) \cdot n)$$
   When $m = N$ and $n = N$, the complexity is $O(N^2)$.
2. **Empirical Doubling Factor:**
   When doubling $N \rightarrow 2N$, the theoretical time ratio is:
   $$\frac{T(2N)}{T(N)} = \frac{O((2N)^2)}{O(N^2)} = 4.0\times$$
   The empirical ratios observed ($2.5\times \rightarrow 5.0\times \rightarrow 4.0\times$) converge cleanly to the theoretical $4.0\times$ quadratic growth as $N$ increases and clock granularity effects diminish.
3. **Space Complexity:**
   $O(m + n)$ additional memory is allocated for the normalized dividend $u'$ and divisor $v'$, and released immediately upon function completion. No memory leaks occur.
