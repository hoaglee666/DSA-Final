# Week 1 Report — Understand & Plan

**Project:** Big-int Division & Modular Exponentation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Project family:** NUMERIC

**Programming language:** C

## 1. About the problem [Shared]

Standard computer structures reply on fixed-width integer (`int, long long`), overflow when dealing with cryptographic numbers consists of hundreds or thousands of digits.

Therefore, our group want to implement a BigInt library in `C`, which requires:

1. Storing large numbers as dynamic arrays of limbs (e.g., base $10^4$ or $2^32$).
2. Addition, substraction, and multiprecision multiplication.
3. Multi digit division with a quotient and remainder using Knuth's algorithm D.
4. Fast modular exponentitation ($base^{exponent} \pmod{m}$) using square and multiply algorithm.
5. Benchmarking & validation by comparing modular exponentiation against naive multiplication.

## 2. Approach and data structures [Shared]

Data structure: dynamic array of limbs (`uint32_t *digits`) wrapped inside a `struct BigInt` that track size, capacity, and sign.

Since contiguous array provide `O(1)` random access by index, Knuth's algorithm D is good with it for inspect leading limbs directly ($u[j + n], v[n - 1]$). The layout of it maximizes CPU cache locality during sequential limb addtition and multiplication.

Base $10^4$ allows intermediate products ($b^2 = 10^8$) to fit with standard 32-bit/64-bit without overflow.

## 3. Syllabus Mapping [Shared]

- Lesson 2 (algorithm complexity & data types): big-O analysis of $O(L^2)$ multiplication, $O(m \cdot n)$ divison, $O(k \cdot L^2)$ modular exponentiation.
- Lesson 6-7 (pointers, dynamic memory & arrays): memory allocation, pointer arithmetic, safety discipline.
- Numerical & D-n-C algorithms: halve recurrences in binary exponentiation and loop invariants in multiprecision arithmetic.

## 4. Hand-trace examples [Shared]

### 4.1. Small example [Shared]

_Task_: verify the correctness of modular reduction property $(a \times b) \pmod m = ((a \pmod m) \times (b \pmod m)) \pmod m$ using 4 step miniature computation: evaluating $7^3 \pmod{11}$

_Hand trace steps_:

1. initalize: base = 7, exponent = 3, modulus = 11.
2. step 1 ($i = 1$): $7^1 = 7 \pmod{11} = 7$.
3. step 2 ($i = 2$): $7^2 = 7 \times 7 = 49 \implies 49 \pmod{11} = 5$.
4. step 3 ($i = 3$): $7^3 = 7 \times 5 = 35 \implies 35 \pmod{11} = 2$.
5. check by computing full power: $7^3 = 343 \implies 343 \pmod{11} = 2$

### 4.2. Fast modular exponentiation [Hoàng]

- Assigned: compute $7^13 \pmod{11}$

- binary: $13_{10}$ has 4 binary bits: $1101_2$
- init: accumulator r = 1, base $b = 7 \pmod{11} = 7$
- step 1 (bit = 1, MSB): square $r^2 \pmod{11} = 1^2 = 1$; multi by base $\rightarrow 1 \times 7 = 7 \pmod{11} \rightarrow \mathbf{r = 7}$
- step 2 (bit = 1): square $r^2 \pmod{11} = 7^2 = 49 \equiv 5 \pmod{11}$; multi by base $\rightarrow 5 \times 7 = 35 \equiv 2 \pmod{11} \rightarrow \mathbf{r = 2}$
- step 3 (bit = 0): square $r^2 \pmod{11} = 2^2 = 4$; skip multi (bit = 0) $\rightarrow \mathbf{r = 4}$
- step 4 (bit = 1, LSB): square $r^2 \pmod{11} = 4^2 = 16 = 5 \pmod{11}$; multi by base $\rightarrow 5 \times 7 = 35 = 2\pmod{11} \rightarrow \mathbf{r = 2}$
- final: 2

### 4.3. Long division [Phúc]

- Assigned: compute $95432 \div 187$ in base 10.

- parameters: divisor limbs $n = 3 (187)$, dividend $m = 2 (95432)$, quotient digits = $m + 1 = 3$.
- normalization factor: $d = \lfloor10 / (1 + 1)\rfloor = 5$, normalized divisor $v' = 187 \times 5 = 935$, normalized dividend $u' = 95432 \times 5 = 477160$.
- step 1 (iteration $j = 2$, window `4771`):
  - estimate quotient (use leading digits) $\hat{q} = \lfloor47 / 9\rfloor = 5$, remainder estimate $\hat{r} = 47 - (5 \times 9) = 2$.
  - check estimate $\hat q \times v'_{n-1} \leq 10\hat r+u_{j+n-2} \rightarrow 5 \times 3 \leq 10 \times 2 + 7 = 15 \leq 27$. multiply by 10 because base 10.
  - multiply and subtract: $4771 - (5 \times 935) = 4771 - 4675 = 96$.
  - quotient digit $q_2 = 5$.
- step 2 (iteration $j = 1$, window `0966`, bring down 6):
  - estimate quotient (use leading digits) $\hat{q} = \lfloor9 / 9\rfloor = 1$, remainder estimate $\hat{r} = 9 - (1 \times 9) = 0$.
  - check estimate $\hat q \times v'_{n-1} \leq 10\hat r+u_{j+n-2} \rightarrow 1 \times 3 \leq 10 \times 0 + 6 = 3 \leq 6$. multiply by 10 because base 10.
  - multiply and subtract: $966 - (1 \times 935) = 966 - 935 = 31$.
  - quotient digit $q_1 = 1$.
- step 3 (iteration $j = 0$, window `0310`, bring down 0):
  - estimate quotient (use leading digits) $\hat{q} = \lfloor3 / 9\rfloor = 0$, remainder estimate $\hat{r} = 3 - (0 \times 9) = 3$.
  - skip check estimate.
  - multiply and subtract: $310 - (0 \times 935) = 310$.
  - quotient digit $q_0 = 0$.
- final: quotient digits $(5, 1, 0) \rightarrow q = 510$. remainder 310 unnormalized: $310 \div 5 = r = 62$
- check: $510 \times 187 + 62 = 95370 + 62 = 95432$

## 5. Test plan [Shared]

- unit testing: individual test of addition carries, subtraction borrows, and multiplication cross-products.
- algorithm D validation: edge cases include $u \le v, v = 1, v = 0$, inputs where Knuth's $\hat{q}$ correction and add-back steps activate.
- modexp benchmarking: timing execution across 16, 64, 256, 1024 bit exponents to test logarithmic scaling compared to naive multiplication.

## 6. Teach-back [Shared]

### 6.1. Teach-back: modular exponentiation & representation [Hoàng]

- Trace summary: by turning 13 to binary `1101`, 4 bits can be processed from most to least significant. Each bit triggers square operation on the accumulator, when bit is 1, perform multiplication by base. Reduce module m at every step keep number small without changing final remainder.
- Why this DSA: storing limbs in little-endian order help arithmetic loops start at index 0 (LSD), same with how carries propagate. Square and multiply calculates large powers efficiently without multiplying base repeatedly (multiply only when bit is 1). Reduce the number of operations.
- Syllabus topics needed: arrays & pointers (L6-L7), big-O complexity (L2), D-n-C recurrences.

### 6.2. Teach-back: modular exponentiation & representation [Phúc]

- Trace summary: first normalize divisor and dividend by factor d that leading divisor limb is $\leq \lfloor b / 2 \rfloor$. Each iteration j, estimate a trial quotient with leading limbs of the remainder window divided by the leading limbs of divisor, then test if needed and multiply and subtract.
- Why this DSA: contiguous arrays help direct index access, make it easy to test current division window, estimate quotient digits, and multiplication and subtraction. This is good for high speed multiprecision numerial computations.
- Syllabus topics needed: array indexing & memory layout, numerical algorithms, and loop invariants (L2 & L6).

## 7. AI log

| #   | Date       | Student | Tool   | Prompt                                                                | Output                                                                     | Verification                         | Changed                                             |
| --- | ---------- | ------- | ------ | --------------------------------------------------------------------- | -------------------------------------------------------------------------- | ------------------------------------ | --------------------------------------------------- |
| 1   | 2026-10-20 | Hoàng   | Gemini | Provided rubric & project brief, week 1 report structure into 2 roles | Week 1 draft with problem restatement, traces, test plan, syllabus mapping | Manually verified arithmetric traces | Aligned terminology with C dynamic memory standards |
