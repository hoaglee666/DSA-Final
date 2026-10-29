# Data Structure and Algorithms - Final Project

**Project:** Big-int Division & Modular Exponentiation  
**Group:** 22110062 - Đào Nguyên Phúc & 22110026 - Lê Huy Hoàng

---

## Progress Overview

- [x] **Week 1:** Problem understanding, mathematical hand traces (Modexp & Algorithm D), syllabus mapping, and test planning. See [WEEKLY_REPORT_1.md](WEEKLY_REPORT_1.md).
- [x] **Week 2:**
  - **Hoàng:** `BigInt` lifecycle (`create_bigint`, `free_bigint`, `string_to_bigint`, `bigint_to_string`), addition (`bigint_add`), subtraction (`bigint_sub`), multiplication (`bigint_mul`), modular exponentiation (`big_modexp`, `big_modexp_naive`).
  - **Phúc:** Knuth's Algorithm D long division (`big_divmod`), helper functions (`big_div`, `big_mod`), Week 1 trace verification ($95432 \div 187$), automated oracle test suite (700 tests), and empirical complexity analysis. See [WEEKLY_REPORT_2.md](WEEKLY_REPORT_2.md).

---

## Build & Run

To compile and run the full test and benchmark suite using GCC:

```bash
# Compile with optimizations and full warnings
gcc -Wall -Wextra -O2 main.c bigint_hoang.c bigint_phuc.c -o final.exe

# Execute test suite and empirical benchmark
./final.exe

# Run valgrind
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./final
```
