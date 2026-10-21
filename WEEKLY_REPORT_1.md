# Week 1 Report — Understand & Plan

**Project:** Big-int Division & Modular Exponentiation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Repository & Evidence Links:**

- **Public GitHub Repository:** [https://github.com/hoaglee666/DSA-Final.git]
- **Report File Link:** [https://github.com/hoaglee666/DSA-Final.git/blob/main/WEEKLY_REPORT_1.md]
- **Initial Commit Link:** [https://github.com/hoaglee666/DSA-Final/commit/36e29b5b97b0586e638667820f62a2990ba7ca0a]

**Project family:** NUMERIC

**Programming language:** C

## 1. About the problem [Shared]

Standard computer structures rely on fixed-width integer types (`int`, `long long`), which overflow when dealing with cryptographic numbers consisting of hundreds or thousands of digits.

Therefore, our group wants to implement a BigInt library in `C`, which requires:

1. Storing large numbers as dynamic arrays of limbs (e.g., base $10^4$ or $2^{32}$).
2. Addition, subtraction, and multi-precision multiplication.
3. Multi-digit division with a quotient and remainder using Knuth's Algorithm D.
4. Fast modular exponentiation ($base^{exponent} \pmod{m}$) using the square-and-multiply algorithm.
5. Benchmarking & validation by comparing modular exponentiation against naive multiplication.

## 2. Approach and data structures [Shared]

Data structure: dynamic array of limbs (`uint32_t *digits`) wrapped inside a `struct BigInt` that tracks size, capacity, and sign.

Since contiguous arrays provide $O(1)$ random access by index, Knuth's Algorithm D works well with them to inspect leading limbs directly ($u[j + n], v[n - 1]$). The layout maximizes CPU cache locality during sequential limb addition and multiplication.

Base $10^4$ allows intermediate products ($b^2 = 10^8$) to fit within standard 32-bit/64-bit machine words without overflow.

## 3. Syllabus mapping [Shared]

- Lesson 2 (algorithm complexity & data types): Big-O analysis of $O(L^2)$ multiplication, $O(m \cdot n)$ division, $O(k \cdot L^2)$ modular exponentiation.
- Lesson 6-7 (pointers, dynamic memory & arrays): memory allocation, pointer arithmetic, safety discipline.
- Numerical & D-n-C algorithms: halve recurrences in binary exponentiation and loop invariants in multi-precision arithmetic.

## 4. Hand trace example [Shared]

### 4.1. Small example [Shared]

_Task_: Verify the correctness of the modular reduction property $(a \times b) \pmod m = ((a \pmod m) \times (b \pmod m)) \pmod m$ using a 4-step miniature computation: evaluating $7^3 \pmod{11}$.

_Hand trace steps_:

1. Initialize: base = 7, exponent = 3, modulus = 11.
2. Step 1 ($i = 1$): $7^1 = 7 \pmod{11} = 7$.
3. Step 2 ($i = 2$): $7^2 = 7 \times 7 = 49 \implies 49 \pmod{11} = 5$.
4. Step 3 ($i = 3$): $7^3 = 7 \times 5 = 35 \implies 35 \pmod{11} = 2$.
5. Check by computing full power: $7^3 = 343 \implies 343 \pmod{11} = 2$.

### 4.2. Fast modular exponentiation [Hoàng]

- Assigned: Compute $7^{13} \pmod{11}$.

- Binary: $13_{10}$ has 4 binary bits: $1101_2$.
- Init: Accumulator $r = 1$, base $b = 7 \pmod{11} = 7$.
- Step 1 (bit = 1, MSB): Square $r^2 \pmod{11} = 1^2 = 1$; multiply by base $\rightarrow 1 \times 7 = 7 \pmod{11} \rightarrow \mathbf{r = 7}$.
- Step 2 (bit = 1): Square $r^2 \pmod{11} = 7^2 = 49 \equiv 5 \pmod{11}$; multiply by base $\rightarrow 5 \times 7 = 35 \equiv 2 \pmod{11} \rightarrow \mathbf{r = 2}$.
- Step 3 (bit = 0): Square $r^2 \pmod{11} = 2^2 = 4$; skip multiply (bit = 0) $\rightarrow \mathbf{r = 4}$.
- Step 4 (bit = 1, LSB): Square $r^2 \pmod{11} = 4^2 = 16 \equiv 5 \pmod{11}$; multiply by base $\rightarrow 5 \times 7 = 35 \equiv 2 \pmod{11} \rightarrow \mathbf{r = 2}$.
- Final: 2.

### 4.3. Long division [Phúc]

- Assigned: Compute $95432 \div 187$ in base 10.

- Parameters: Divisor limbs $n = 3$ ($187$), dividend $m = 2$ ($95432$), quotient digits = $m + 1 = 3$.
- Normalization factor: $d = \lfloor10 / (1 + 1)\rfloor = 5$, normalized divisor $v' = 187 \times 5 = 935$, normalized dividend $u' = 95432 \times 5 = 477160$.
- Step 1 (iteration $j = 2$, window `4771`):
  - Estimate quotient (use leading digits) $\hat{q} = \lfloor47 / 9\rfloor = 5$, remainder estimate $\hat{r} = 47 - (5 \times 9) = 2$.
  - Check estimate $\hat q \times v'_{n-1} \leq 10\hat r+u_{j+n-2} \rightarrow 5 \times 3 \leq 10 \times 2 + 7 = 15 \leq 27$. Multiply by 10 because base 10.
  - Multiply and subtract: $4771 - (5 \times 935) = 4771 - 4675 = 96$.
  - Quotient digit $q_2 = 5$.
- Step 2 (iteration $j = 1$, window `0966`, bring down 6):
  - Estimate quotient (use leading digits) $\hat{q} = \lfloor9 / 9\rfloor = 1$, remainder estimate $\hat{r} = 9 - (1 \times 9) = 0$.
  - Check estimate $\hat q \times v'_{n-1} \leq 10\hat r+u_{j+n-2} \rightarrow 1 \times 3 \leq 10 \times 0 + 6 = 3 \leq 6$. Multiply by 10 because base 10.
  - Multiply and subtract: $966 - (1 \times 935) = 966 - 935 = 31$.
  - Quotient digit $q_1 = 1$.
- Step 3 (iteration $j = 0$, window `0310`, bring down 0):
  - Estimate quotient (use leading digits) $\hat{q} = \lfloor3 / 9\rfloor = 0$, remainder estimate $\hat{r} = 3 - (0 \times 9) = 3$.
  - Skip check estimate.
  - Multiply and subtract: $310 - (0 \times 935) = 310$.
  - Quotient digit $q_0 = 0$.
- Final: Quotient digits $(5, 1, 0) \rightarrow q = 510$. Remainder 310 unnormalized: $310 \div 5 = r = 62$.
- Check: $510 \times 187 + 62 = 95370 + 62 = 95432$.

## 5. Test plan [Shared]

- Unit testing: Individual test of addition carries, subtraction borrows, and multiplication cross-products.
- Algorithm D validation: Edge cases include $u \le v, v = 1, v = 0$, inputs where Knuth's $\hat{q}$ correction and add-back steps activate.
- Modexp benchmarking: Timing execution across 16, 64, 256, 1024 bit exponents to test logarithmic scaling compared to naive multiplication.

## 6. Week 2 plan[Shared]

### 6.1. Hoàng's tasks (Target: $\ge 8$ hours)

1. Work on `struct BigInt`, dynamic memory allocation `malloc`/`free`, string to BigInt parsing, BigIn to string formatting.
2. Addition `big_add` and subtraction `big_sub` with carry and borrow.
3. Multiplication `big_mul` ($O(L^2)$ limb complexity).
4. Modular exponentiation `big_modexp` with square and multiply and naive modular exponentiation `big_modexp_naive`.
5. Write unit tests on week 1 modexp trace ($7^{13} \pmod{11}$).

### 6.2. Phúc's tasks (Target: $\ge 8$ hours)

1. Long division `big_divmod` use normalization, quotient estimation, correction steps.
2. Write unit tests on week 1 division trace ($95432 \div 187$).
3. Create oracle test to validate division and arithmetic results automatically.
4. Draft initial empirical complexity notes for division.

## 7. Teach back [Shared]

### 7.1. Modular exponentiation & representation [Hoàng]

- Trace summary: By turning 13 to binary `1101`, 4 bits can be processed from most to least significant. Each bit triggers a square operation on the accumulator; when a bit is 1, perform multiplication by the base. Reducing modulo $m$ at every step keeps numbers small without changing the final remainder.
- Why this DSA: Storing limbs in little-endian order helps arithmetic loops start at index 0 (LSD), matching how carries propagate. Square and multiply calculates large powers efficiently without multiplying the base repeatedly (multiplying only when the bit is 1). This reduces the number of operations. If naively multiplying the base repeatedly, it takes $O(k)$ multiplications for an exponent with value $k$; square and multiply takes $O(\log k)$ or $O(L^2 \log k)$ time when each multiplication on $L$-limb integers uses ordinary multiplication. If the exponent has $k$ binary bits, this is $O(kL^2)$.
- Syllabus topics needed: Arrays & pointers (L6-L7), Big-O complexity (L2), D-n-C recurrences.

### 7.2. Teach-Back: Long Division [Phúc]

- Trace summary: First normalize the divisor and dividend by factor $d$ so that the leading divisor limb is $\le \lfloor b / 2 \rfloor$. In each iteration $j$, estimate a trial quotient with the leading limbs of the remainder window divided by the leading limbs of the divisor, then test if needed and multiply and subtract.
- Why this DSA: Contiguous arrays help direct index access, making it easy to test the current division window, estimate quotient digits, and perform multiplication and subtraction. This is ideal for high-speed multi-precision numerical computations. Time complexity is $O(mn)$, where $m$ is the number of extra dividend limbs and $n$ is the number of divisor limbs.
- Syllabus topics needed: Array indexing & memory layout, numerical algorithms, and loop invariants (L2 & L6).

## 8. AI Log

| #   | Date       | Student | Tool   | Prompt                                                                | Output                                                                     | Verification                        | Changed                                             |
| --- | ---------- | ------- | ------ | --------------------------------------------------------------------- | -------------------------------------------------------------------------- | ----------------------------------- | --------------------------------------------------- |
| 1   | 2026-10-20 | Hoàng   | Gemini | Provided rubric & project brief, week 1 report structure into 2 roles | Week 1 draft with problem restatement, traces, test plan, syllabus mapping | Manually verified arithmetic traces | Aligned terminology with C dynamic memory standards |
