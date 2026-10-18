# Week 1 Report — Self-Organizing Linked Lists

**Project family:** LIST  
**Programming language:** C  
**Week 1 theme:** Understand & Plan  
**Status:** Initial design and test plan

## 1. About the problem

Linked lists store elements in nodes, connected by pointers. For singly linked lists, nodes contain data and pointers to next node. List is accessed through head pointer, final node points to `NULL`.

In worst-case, searching takes `(O(n))`, `(n)` is number of nodes. If this value is accessed more than that value, keeping list in its originall order result in searching the same nodes repeatedly.

Our group want to see if changing the order of nodes after successful searches reduce search cost for some access patterns, by self-organizing linked lists.

We will have a singly linked list, and 2 self-organizing methods:

1. **move-to-front (MTF):** Successful search, move accessed node to head of list.
2. **count-based:** Successful accesses got tracked, then organize list in descending order of access count.

Ordinary list, with the same access sequences and number of nodes for all methods.

## 2. Approach and data structures

### 2.1 Singly linked list

The core is singly linked list written in `C`.

Basic node has:

- `data`: integer stored in node.
- `next`: pointer to next node.

List will have a `head` pointer, with `tail` pointer and `size` counter for the extended methods.

Nodes will store access count in **count-based**.

### 2.2 Planned structure

```c
typedef struct Node {
    int data;
    struct Node *next;
    unsigned long access_count;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int size;
} List;
```

`access_count` is for **count-based**, `tail` pointer, `size` counter will be used for insertion, deletion, and reorganization.

Integer as stored value since it is easy to trace by hand, help focus on linked list behavior.

### 2.3 Planned operations

- Init empty list.
- Insert a node at head.
- Insert a node at tail.
- Search for a value with sequential traversal.
- Delete a node by value.
- Display the list.
- Free nodes.
- Maintain `tail` pointer and `size` counter correctly.
- Self-organize after successful searches.

### 2.4 Why this approach

Nodes can be reordered by changing pointers instead of shifting array of elements in linked lists. In **move-to-front**, nodes can be moved to head by adjusting links.

But linked lists don't provide constant time accessed. Search by value needs sequential traversal. So, worst-case `(O(n))` search is not removed, but number of nodes examined can be reduced for some access patterns.

## 3. Hand Trace: move-to-front

### 3.1 Initial list

The initial list is:

```text
head
 |
 v
[A] -> [B] -> [C] -> [D] -> [E] -> [F] -> NULL
```

Access sequence:

```text
C, E, C, C
```

Searchs start at head. Search cost measured by counting nodes examined, include target node. If success, accessed node move to head.

### 3.2 Trace table

| Step    | Accessed value | Nodes examined | List after the operation   |
| ------- | -------------- | -------------: | -------------------------- |
| Initial | —              |              — | A -> B -> C -> D -> E -> F |
| 1       | C              |              3 | C -> A -> B -> D -> E -> F |
| 2       | E              |              5 | E -> C -> A -> B -> D -> F |
| 3       | C              |              2 | C -> E -> A -> B -> D -> F |
| 4       | C              |              1 | C -> E -> A -> B -> D -> F |

Lists end with `NULL`, keep rows compact.

### 3.3 Explanation for steps

**Step 1: Search for C**

Start at A, then B, then C. Search cost is 3 nodes visit. Since C is not at head, moved to front.

```text
Before: A -> B -> C -> D -> E -> F -> NULL
After:  C -> A -> B -> D -> E -> F -> NULL
```

**Step 2: Search for E**

Now at C, A, B, D, and E. Cost is 5 nodes. Then E moved to front.

```text
Before: C -> A -> B -> D -> E -> F -> NULL
After:  E -> C -> A -> B -> D -> F -> NULL
```

**Step 3: Search for C**

E then C, cost is 2 nodes. C to the front again.

```text
Before: E -> C -> A -> B -> D -> F -> NULL
After:  C -> E -> A -> B -> D -> F -> NULL
```

**Step 4: Search for C again**

C is already at head. so cost is 1 node. Order does not change.

### 3.4 Comparison with an ordinary list

If list is just in its orginal order always, search costs would be:

| access                             | normal list | move-to-front |
| ---------------------------------- | ----------: | ------------: |
| C                                  |           3 |             3 |
| E                                  |           5 |             5 |
| C                                  |           3 |             2 |
| C                                  |           3 |             1 |
| **Total node visits**              |      **14** |        **11** |
| **Average node visits per search** |     **3.5** |      **2.75** |

In this one, **move-to-front** reduces number of node visits from 14 to 11.

This does not mean **move-to-front** always faster. Different access sequences produce different results.

## 4. count-based method

Nodes will store access count, initially 0. When a search is successful, count increased by 1. Nodes will be ordered as nodes with larger counts before smaller ones.

Ties will keep the relative order of nodes with equal counts.

## 5. Test plan

| test | operation                                     | expected result                                                |
| ---- | --------------------------------------------- | -------------------------------------------------------------- |
| T1   | Initialize a list                             | head and tail are `NULL`; size is zero                         |
| T2   | Insert at head                                | 1 node; head and tail point to it                              |
| T3   | Insert several values at tail                 | Values show up in inserted order                               |
| T4   | Search for head value                         | Search succeeds after 1 node visit                             |
| T5   | Search for a middle value                     | Search returns correct node                                    |
| T6   | Search for a missing value                    | Search not found without invalid pointer access                |
| T7   | Delete head by value                          | head advances correctly; size decreases                        |
| T8   | Delete a middle value                         | Predecessor links to the deleted node's successor              |
| T9   | Delete tail by value                          | tail becomes the new final node; its `next` is `NULL`          |
| T10  | Delete the only node                          | head and tail become `NULL`; size is 0                         |
| T11  | Delete a value not present                    | List not changed                                               |
| T12  | Insert at tail with a maintained tail pointer | New node is appended and tail is updated                       |
| T13  | Repeated move-to-front searches               | Accessed nodes move to the head without losing nodes           |
| T14  | Repeated count-based searches                 | Counts increase correctly and ordering follows the stated rule |
| T15  | Compare methods on same workload              | Node visits and average search costs are recorded consistently |
| T16  | Free list                                     | All allocated nodes are released and list pointers are reset   |

## 6. Syllabus mapping

| syllabus topic                             | source  | usages                                                                                |
| ------------------------------------------ | ------- | ------------------------------------------------------------------------------------- |
| Linked lists                               | L06     | Node structure, head pointer, insertion, deletion, sequential search, pointer updates |
| Linked list implementation and testing     | L07     | C implementation, test driver, edge cases, tracing, and debugging                     |
| Complexity analysis                        | L02     | Big-O notation, counting basic operations, cost grows with input size                 |
| List operations and sequential search cost | L06–L07 | Implement operations and count nodes examined by searches                             |
| Expected cost                              | L02     | Compare search costs under different workloads                                        |

## 7. AI log

| prompt                                                                          | purpose                                      | output                                                                      |
| ------------------------------------------------------------------------------- | -------------------------------------------- | --------------------------------------------------------------------------- |
| read the rubric, the pdf files and help me set up a plan to do my final project | Setting up project scope                     | Singly linked list with integers, with self-organizing methods              |
| check if contents of the plan correct with component C                          | Check required concept based on PDF files    | Map list operations, sequential search cost, expected cost to project       |
| is the contents based on the supplied pdfs                                      | Avoid silently introducing untaught material | Use L02, L06, and L07 as primary references; flag unconfirmed as extensions |
| hand trace example for week 1                                                   | Meet the requirement in rubric               | Trace six nodes under move-to-front                                         |

## 8. References

1. **Lesson 2 — Overview: From Problem to Program, Data Types, Algorithm Complexity.** Pointers, dynamic memory, operation counting, and Big-O notation.
2. **Lesson 6 — Lists Continued: Linked Lists, Singly Linked List Operations.** Node structures, insertion, deletion, sequential search, pointer correctness, and complexity comparisons.
3. **Lesson 7 — Practice Lab: Building and Using Singly Linked Lists.** Checklist, test driver, edge-case testing, and debugging guidance.
4. **Project rubric — Appendix A and Appendix B.** Syllabus mapping and LIST-family concept questions and live-modification tasks.

**Week 1 conclusion:** Singly linked list written in `C` will be used.
