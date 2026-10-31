# Week 3 Report — Experiments & Analysis

**Project:** Big-int Division & Modular Exponentiation | DSA Final Project

**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

**Repository & Evidence Links:**

- **Public GitHub Repository:** [https://github.com/hoaglee666/DSA-Final](https://github.com/hoaglee666/DSA-Final)
- **Week 3 Target Commit:** [`1319396972`](https://github.com/hoaglee666/DSA-Final/commit/d8af94af8a359ad8f30948ac75d5cb63f8d42021)
- **Week 1 Report:** [WEEKLY_REPORT_1.md](WEEKLY_REPORT_1.md)
- **Week 2 Report:** [WEEKLY_REPORT_2.md](WEEKLY_REPORT_2.md)
- **Source Files (direct links):**
  - [`bigint.h`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint.h) — BigInt struct definition and function prototypes
  - [`bigint_hoang.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_hoang.c) — Memory lifecycle, add/sub/mul, naive & fast modexp
  - [`bigint_phuc.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_phuc.c) — Knuth Algorithm D, oracle test suite, benchmarks
  - [`main.c`](https://github.com/hoaglee666/DSA-Final/blob/main/main.c) — Test and benchmark runner

---

## 1. Workload [Shared]

The tasks completed in Week 2 match the plan we laid out in Week 1:

| Member              | Tasks Completed                                                                                       | Status   | Evidence                                                                                         |
| :------------------ | :---------------------------------------------------------------------------------------------------- | :------- | :----------------------------------------------------------------------------------------------- |
| **Đào Nguyên Phúc** | Save logs for 16/64/256/1024 bit tests, optimize Knuth's algorithm D, finish oracle tests             | **Done** | [`bigint_phuc.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_phuc.c), `main.c`     |
| **Lê Huy Hoàng**    | Benchmark to compare modular exponentiation methods, oracle randomized tests, Valgrind to check leaks | **Done** | [`bigint_hoang.c`](https://github.com/hoaglee666/DSA-Final/blob/main/bigint_hoang.c), `bigint.h` |

---

## 2. Results & performance [Shared]

### 2.1. Modular exponentiation [Hoàng]

Square-and-multiply $O(\log{e})$ `big_modexp` compared to naive $O(e)$ `big_modexp_naive` with base = 123 and modulo = 10007. Results differ each runtime.

```plaintext
=== FAST VS. NAIVE MODEXP BENCHMARK ===
EXPONENT   | FAST MODEXP (ms)   | NAIVE MODEXP (ms)  | SPEEDUP RATIO
--------------------------------------------------------------
10         | 0.004              | 0.004              | 1.00
100        | 0.005              | 0.030              | 6.00
500        | 0.028              | 0.179              | 6.39
1000       | 0.007              | 0.315              | 45.00
5000       | 0.008              | 1.798              | 224.75
10000      | 0.008              | TIMEOUT (>5s)      | N/A (O(e))
=== MODEXP BENCHMARK COMPLETED ===
```

- Fast modular: Runtime almost the same $\approx 0.004 - 0.028$, going from 10 to 10000 exponent. Prove time complexity $O(\log_2{e})$.
- Naive modular: Linear $O(e)$ based on exponent and take more than 5s, which cause timeout at $e = 10000$.

### 2.2. Arithmetic oracle [Hoàng]

To check correctness of `bigint_mul`, oracle was implemented to generate 1000 64-bit randomized tests and compare to `__int128`.

```plaintext
=== ARITHMETIC ORACLE TESTING (vs. __int128) ===
ORACLE TEST RESULTS: 1000 / 1000 RANDOMIZED MULTIPLICATION TESTS PASSED SUCCESSFULLY.
=== ORACLE TESTING COMPLETED===
```

### 2.3. Algorithm D analysis [Phúc]

Average runtime `big_divmod` calculated with limb $N$ increase from 10 to 640.

```plaintext
=== EMPIRICAL COMPLEXITY ANALYSIS FOR KNUTH ALGORITHM D ===
Measuring division runtime across scaling limb sizes:
Limbs (N)  | Digits (~4N) | Iterations      | Avg Time (us)   | T(N)/T(N/2)
----------------------------------------------------------------------
10         | 40           | 500             | 1.37            | -
20         | 80           | 500             | 5.22            | 3.80x
40         | 160          | 500             | 17.04           | 3.27x
80         | 320          | 100             | 84.67           | 4.97x
160        | 640          | 100             | 381.98          | 4.51x
320        | 1280         | 20              | 1472.35         | 3.85x
640        | 2560         | 5               | 5805.60         | 3.94x
----------------------------------------------------------------------
Theoretical Knuth Algorithm D complexity: O(m * n).
When doubling both m and n (N -> 2N), expected theoretical growth is 4.00x (O(N^2)).
=== BENCHMARK COMPLETED ===
```

### 2.4. Timing logs for (`big_modexp`) [Phúc]

```plaintext
=== WEEK 3 MODEXP BENCHMARK EXECUTION LOG ===
Compiler: GCC -O2, Target: Windows x86_64

--- [16-bit Benchmark Input Vectors] ---
Base (5 digits): 99496
Exp  (5 digits): 63988
Mod  (5 digits): 72071
Result preview: 59743...
Iterations: 1000, Total Time: 0.0080 s, Avg Time: 8.00 us (Scaling: -)

--- [64-bit Benchmark Input Vectors] ---
Base (20 digits): 86461350924851857058
Exp  (20 digits): 20205077128994077453
Mod  (20 digits): 94889611738135745419
Result preview: 15887027393925014088...
Iterations: 500, Total Time: 0.0210 s, Avg Time: 42.00 us (Scaling: 5.25x)

--- [256-bit Benchmark Input Vectors] ---
Base (78 digits): 732764868426772758684964217441811301963635099490565211024683619565239924146544
Exp  (78 digits): 330644830302724166839929671957271527515087258758260002162596407920167131161064
Mod  (78 digits): 752678452538767010709793718845433616734927944702894647370473259772422761672037
Result preview: 627883760501284767251940685655485370529628493138050005949079016...
Iterations: 100, Total Time: 0.0800 s, Avg Time: 800.00 us (Scaling: 19.05x)

--- [1024-bit Benchmark Input Vectors] ---
Base (309 digits): 499266451094371998219652087200974121956328429347486069704001099768712214581829792884420645209964806584308812305191585448693672296716175690856766935258474159073437477243298526580352703146811300997422226256861552233757459533267998835567892632628414256086376943378150158749492511657421310571861468848545759279218
Exp  (309 digits): 520747543337921615211563474097920061825016725277770017980865546213980571065991084395967976269211880917148896612654896795069527325558051525756097146366007876690849811764388319370881271942646385981943079850610334430354781753238119339745701697193372815929100322531722815735355836865865542247254931409251735707421
Mod  (309 digits): 254601130821947341474469753518354898321122475992842044919103464982186313266412770707951165735930049452720384192324111537256087295686744816825755625018403870471683832054378998537881419281085246285466002672229431123808309701599028025534087247904777079293744160071558353187534526042022876797846086257581362192702
Result preview: 155063611981383443904205376140515972691615201602899827412176746...
Iterations: 10, Total Time: 0.4080 s, Avg Time: 40800.00 us (Scaling: 51.00x)

=== SUMMARY & SPEEDUP ===
16-bit Naive Time: 1200.00 us, Fast Time: 8.00 us (Speedup: 150.0x)

```

### 2.5. Valgrind check [Shared]

Use Valgrind on program to check for leak:

```bash
valgrind --leak-check=full --show-leak-kinds=all --show-origins=yes ./final
```

```plaintext
==3521==
==3521== HEAP SUMMARY:
==3521==     in use at exit: 0 bytes in 0 blocks
==3521==   total heap usage: 46,748 allocs, 46,748 frees, 1,981,318 bytes allocated
==3521==
==3521== All heap blocks were freed -- no leaks are possible
==3521==
==3521== For lists of detected and suppressed errors, rerun with: -s
==3521== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## 3. AI Log [Shared]

| Phase  | Member | Prompt / Suggestion                        | AI Output                                                                                      | Bug Encountered           | How We Fixed It                                                   |
| :----- | :----- | :----------------------------------------- | :--------------------------------------------------------------------------------------------- | :------------------------ | :---------------------------------------------------------------- |
| Week 3 | Hoàng  | got too many arguments error for my oracle | `printf("MISMATCH! %s \* %s -> Got %s, Expected %s\n", buf1, buf2, buf_result, expected_buf);` | Typo `$s` instead of `%s` | Change `$s` to `%s`                                               |
| Week 3 | Phúc   | how to optimize alg D                      | Create scratchpad before loop                                                                  | -                         | Add scratchpad before loop, verify memory and check with Valgrind |

---

## 5. Week 4 Plan [Shared]

- Finish repository & source code.
- Write final report, include complexity analysis, limitations, future development.
- Oral defense preparation.
