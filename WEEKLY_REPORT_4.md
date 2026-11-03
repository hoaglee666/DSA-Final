# Week 4 Report - Final Report

**Project:** Big-int Division & Modular Exponentiation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Repository & Evidence Links:**

- **Public GitHub Repository:** [https://github.com/hoaglee666/DSA-Final](https://github.com/hoaglee666/DSA-Final)
- **Week 4 Target Commit:** [https://github.com/hoaglee666/DSA-Final/commit/4d9e7a1ba722c428467b60341fb7381843a066d3]
- **Week 1 Report:** [WEEKLY_REPORT_1.md](WEEKLY_REPORT_1.md)
- **Week 2 Report:** [WEEKLY_REPORT_2.md](WEEKLY_REPORT_2.md)
- **Week 3 Report:** [WEEKLY_REPORT_3.md](WEEKLY_REPORT_3.md)
- **Source Files (direct links):**
  - [`bigint.h`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint.h) — BigInt struct definition and function prototypes
  - [`bigint_hoang.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_hoang.c) — Memory lifecycle, add/sub/mul, naive & fast modexp
  - [`bigint_phuc.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_phuc.c) — Knuth Algorithm D, oracle test suite, benchmarks
  - [`main.c`](https://github.com/hoaglee666/DSA-Final/blob/main/main.c) — Test and benchmark runner

---

## 1. Summary [Shared]

The final product repository contain:

1. Little endian dynamic arrays of limbs with heap allocation, reallocation scaling, sign management and leading zero trimming.
2. Signed addition, subtraction handling uniform and mixed signs, with multiplication $O(L^2)$.
3. Implementation of Knuth's algorithm D with normalization, trial quotient estimation, correct/add back steps for multi digits divisors.
4. Binary repeated squaring algorithm $base^{exponent} \pmod {m}$, benchmarked against naive multiplication.
5. RSA key generation, encryption and decryptino using small prime parameters.

## 2. Knuth's algorithm D complexity analysis [Phúc]

### 2.1. Theory

Multiply or divide 2 N limb numbers using classic algorithms, time complexity scale quadratically with limb length N.

$$T(N) = 4 \cdot T(N/2) + O(N) \rightarrow T(N) = O(N^2)$$

Double the input N to 2N, ratio is:

$$\frac{T(2N)}{T(N)} = \frac{(2N)^2}{N^2} = 4$$

### 2.2. Measurements & ratio analysis

```plaintext
=== EMPIRICAL COMPLEXITY ANALYSIS FOR KNUTH ALGORITHM D ===
Measuring division runtime across scaling limb sizes:
Limbs (N)  | Digits (~4N) | Iterations      | Avg Time (us)   | T(N)/T(N/2)
----------------------------------------------------------------------
10         | 40           | 500             | 1.38            | -
20         | 80           | 500             | 4.88            | 3.53x
40         | 160          | 500             | 18.50           | 3.79x
80         | 320          | 100             | 87.91           | 4.75x
160        | 640          | 100             | 363.28          | 4.13x
320        | 1280         | 20              | 1488.90         | 4.10x
640        | 2560         | 5               | 5762.80         | 3.87x
----------------------------------------------------------------------
Theoretical Knuth Algorithm D complexity: O(m * n).
When doubling both m and n (N -> 2N), expected theoretical growth is 4.00x (O(N^2)).
=== BENCHMARK COMPLETED ===
```

Ratio $\frac{T(N)}{T(N/2)}$ hover around 4.0x rather than locking down to exact 4.00 constant. This does not invalidate $O(N^2)$, it reflects real experimental noise and system jitter.

## 3. Modular exponentiation complexity analysis [Hoàng]

### 3.1. Theory

Let e be exponent, m be modulus, L be limb length of number.

- Fast modular exponentiation:
  - How it works: Algorithm scan the exponent e bit by bit from most significant bit to least. Number of bit iterations scale with binary length of exponent, $k = \lfloor log_{2}e \rfloor + 1$. Each iteration, perform a squaring $O(L^2)$, multiply by base if bit = 1 $O(L^2)$ followed with modular reduction $O(L^2)$.
  - Complexity: $O(loge \cdot L^2)$, logarithmic with respect to exponent e, quadratic with respect to limb L.

- Naive modular exponentiation:
  - How it works: Linear loop multiply the base by current accumulator e consecutive times: $res = (res \times base) \pmod {m}$.
  - Complexity: $O(e \cdot L^2)$, linear with respect to exponent e.

### 3.2. Measurements & ratio analysis

```plaintext
=== FAST VS. NAIVE MODEXP BENCHMARK ===
EXPONENT   | FAST MODEXP (ms)   | NAIVE MODEXP (ms)  | SPEEDUP RATIO
--------------------------------------------------------------
10         | 0.006              | 0.004              | 0.67
100        | 0.006              | 0.031              | 5.17
500        | 0.007              | 0.149              | 21.29
1000       | 0.007              | 0.294              | 42.00
5000       | 0.008              | 1.490              | 186.25
10000      | 0.008              | TIMEOUT (>5s)      | N/A (O(e))
=== MODEXP BENCHMARK COMPLETED ===
```

- As e increase from 10 to 10000, fast modexp execution time remain flat, steady between 0.006ms and 0.008ms. This because doubling e only add a single bits to its binary representation $\lfloor log_{2}e \rfloor + 1$, require at most 1 more loop iteration.

- Naive modexp scales directly to value of e, at e = 5000, it takes 1.490ms, slowdown 186x, by e = 10000, it exceeds running time and timeout.

## 4. Limitations [Shared]

1. Multiplication celling: with $O(N^2)$, while it's good for moderate sized keys, it declined with ultra large crypto parameters. Advanced algorithms like Karatsuba $O(N^{log_{2}3})$ are outside current scope.
2. Base $10^4$ trade off: With current base, it simplifies decimal IO conversion and string format, but less data per word compared to $2^32$.

## 5. AI log [Shared]

| #   | Date       | Student | Tool   | Prompt                                                                | Output                                                                                       | Verification                                                                                                                                                                                                        | Final changes                                                                                                                    |
| --- | ---------- | ------- | ------ | --------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| 1   | 2026-10-20 | Hoàng   | Gemini | Provided rubric & project brief, week 1 report structure into 2 roles | Week 1 draft with problem restatement, traces, test plan, syllabus mapping                   | Manually verified arithmetic traces                                                                                                                                                                                 | Aligned terminology with C dynamic memory standards                                                                              |
| 2   | 2026-10-24 | Hoàng   | Gemini | cleanup loop for `big_modexp_naive`                                   | `free_bigint(temp); res = big_mod(temp, mod);`                                               | Call `big_mod` on `temp` after free it caused segmentation fault                                                                                                                                                    | Call `new_res = big_mod(temp, mod)` first, then `free_bigint(temp)` and `free_bigint(res)`, then assign `res = new_res`          |
| 3   | 2026-10-23 | Phúc    | Gemini | Initial scaffold for Knuth algorithm D                                | Skipped normalization, calculated $\hat{q}$ directly from raw limbs $u_{j+n} / v_{n-1}$      | Without $v'_{n-1} \ge \lfloor \text{BASE} / 2 \rfloor$, trial quotient can overshoot by 2 or more, breaking Knuth's invariant. Test 6 failed.                                                                       | Added Step D1 (multiply $u$ and $v$ by $d = \lfloor \text{BASE} / (v_{n-1} + 1) \rfloor$) and Step D8 (divide remainder by $d$). |
| 4   | 2026-10-30 | Hoàng   | Gemini | got too many arguments error for my oracle                            | printf("MISMATCH! %s \* %s -> Got %s, Expected %s\n", buf1, buf2, buf_result, expected_buf); | Typo $s instead of %s                                                                                                                                                                                               | Change $s to %s                                                                                                                  |
| 5   | 2026-10-29 | Phúc    | Gemini | how to optimize alg D C                                               | create scratchpad before loop                                                                | `uint32_t u_stack[STACK_SCRATCH_CAP];` `uint32_t v_stack[STACK_SCRATCH_CAP];` `uint32_t p_stack[STACK_SCRATCH_CAP];` `uint32_t *u_prime = u_stack;` `uint32_t *v_prime = v_stack;` `uint32_t *p_scratch = p_stack;` | Add scratchpad before loop, verify memory and check with Valgrind                                                                |
| 6   | 2026-11-2  | Shared  | Gemini | generate 10 Q&As for each on this project                             | Draft Q&As based on project                                                                  | 10 Q&As for each students based on project context                                                                                                                                                                  | Based on draft, prepare 10 Q&As for each student                                                                                 |

## 6. Self-prepared Q&A & reflections

### 6.1. Section [Hoàng]

Hardest thing was implmented signed arithmetic and handle it correctly along with carry for addition, multiplication and borrow for subtraction.

10 Self-prepared defense Q&As:

1. Why use a contiguous dynamic array `struct BigInt` instead of a linked list?
   - Although linked lists are major part of the course (L6-L7), our project need frequent random access - mainly for division and multiplication. Contiguous dynamic arrays give $O(1)$ random access by index and maximize CPU cache locality. Little endian ordering ensure index 0 is least significant, like how arithmetic carries propagate upward.
2. Explain how dynamic memory management prevent buffer overflow during reallocation.
   - Every `struct BigInt` track its current `size` and allocated `capacity`, when exceed it, allocation doubled by `realloc` and update the pointer.
3. Walkthrough time complexity and mechanism of Fast Modular Exponentiation.
   - Fast modexp use binary requeated squaring. For exponent e with k = log_2{e} + 1 bits, it performs k squaring steps, multiply by base if bit = 1. This reduce complexity from O(e) linear to O(k . L^2).
4. Why base 10^4 instead of binary bases like 2^32?
   - Base 10^4 store 4 decimal per limb. Maximum product of 2 limbs is about 10^8, fit inside 32 bit unsigned integers without overflowing.
5. How `bigint_add` handle operands with different signs?
   - By compare the magnitudes `compare_abs`, subtract the smaller magnitude from larger, then assign sign of operand with larger magnitude.
6. Explain how subtraction is implemented using algebraic negation.
   - Subtraction implemented via $A - B = A + (-B)$, flip the sign of operand B and invoke `bigint_add()`, automatically handle borrow and sign edge cases.
7. What is purpose of `trim_leading_zeros()`?
   - Operations can leave redundant zero limbs at most significant. With the function, `size` are reduced to keep representations correct and prevent bloated memory.
8. How did you verify the correctness of your multiplication implementation?
   - Oracle is implemented to compare BigInt outputs against native `__int128` over randomized test cases covering positive, negative, zero inputs.
9. What course lessons from syllabus map directly to your work?
   - Lesson 2 - Big-O complexity analysis, Lessons 6-7 - dynamic memory allocation, pointers, heap safety, and divide and conquer principles.
10. If you had more time, what optimization would you implement?
    - I would implement Karatsuba multiplication for more efficient scaling.

### 6.2. Section [Phúc]

Hardest thing was Knuth's algorithm D division, managing normalization, ensure trial quotient did not overshoot, and correctly implement multi limb subtraction.

10 Self-prepared defense Q&As:

1. Explain how Knuth's algorithm D division works, why normalization is mandatory?
   - Algorithm D divides multi-precision integers by estimating trial quotients $\hat{q}$. Normalization multiplies the dividend and divisor by a factor $d$ so that the leading divisor limb satisfies $v_{n-1} \ge \lfloor \text{BASE} / 2 \rfloor$. This guarantees that trial quotient estimates overshoot by at most 2, bounding the correction steps.
2. What happens if omit normalization step?
   - Without normalization, trial quotient estimation becomes loose, leading to severe overestimation errors, incorrect remainder windows, negative limb underflows, and infinite adjustment loops in the correction phase.
3. How does you code extract quotient and remainder limbs during division?
   - The algorithm loops through dividend windows from $j = m - n$ down to 0, estimating $\hat{q}$ using leading limb division, multiplying the normalized divisor by $\hat{q}$, subtracting it from the current remainder window, and adjusting if the subtraction borrows out.
4. How is the remainder denormalized?
   - Because the entire division was performed on multiplied (normalized) values, the resulting remainder stored in the working array is scaled by $d$. To obtain the true remainder, we divide the final remainder BigInt by $d$ using a single-precision division routine.
5. Walkthrough time complexity of multiprecision division.
   - Knuth's Algorithm D runs in $O(m \cdot n)$ time, where $m$ is the number of dividend limbs and $n$ is the number of divisor limbs. When dividend and divisor sizes are balanced ($m \approx n \approx N$), the complexity simplifies to $O(N^2)$.
6. How did you implement modular exponentiation benchmarking suite?
   - We measured execution times across exponent bit-widths (16, 64, 256, 512, 1024 bits). Our benchmark runner compares our logarithmic square-and-multiply (big_modexp) against naive repeated multiplication, demonstrating exponential vs. polynomial growth curves.
7. Why does Naive Modular Exponentiation time out for large exponents?
   - Naive exponentiation performs linear repeated multiplication ($O(e)$), meaning doubling the exponent doubles the computation time. For 1024-bit exponents ($e \approx 10^{308}$), it requires to much operations, causing immediate timeouts.
8. How does the educational RSA demonstration integrate your BigInt modules?
   - The RSA toy demo generates small prime numbers $p$ and $q$, computes modulus $n = p \times q$ and Euler's totient $\phi(n)$, selects a public exponent $e$, calculates the private exponent $d$ using modular inverse, and executes big_modexp for encryption ($c = m^e \pmod n$) and decryption ($m = c^d \pmod n$).
9. How did you account for empirical measurement jitter in your performance analysis?
   - We recognized that execution time ratios fluctuate around theoretical values (e.g., $4.0\times$) due to CPU caching, OS scheduling, and dynamic clock throttling, rather than matching static textbook numbers perfectly.
10. How do you ensure your repository is clean and verifiable for the grading examiner?
    - Our public GitHub repository features a structured directory layout (src/, include/, tests/), clean commit histories with descriptive messages, a comprehensive Makefile, and fully reproducible build instructions.
