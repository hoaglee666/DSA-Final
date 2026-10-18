# Week 1 Report — Self-Organizing Linked Lists

**Project family:** LIST  
**Programming language:** C  
**Week 1 theme:** Understand & Plan  
**Status:** Initial design and test plan

## 1. Problem Restatement

A linked list stores elements in nodes connected by pointers. In a singly linked list, each node contains a data value and a pointer to the next node. The list is accessed through a head pointer, and the final node points to `NULL`.

Searching for a value in an ordinary linked list can require visiting many nodes. It takes `(O(n))` time in the worst case, where `(n)` is the number of nodes. If values are accessed more frequently than others, keeping the list in its original state may end up in repeatedly searching through the same preceding nodes.

Therefore, our group plan on implementing a self-organizing linked list in `C` and investigate whether changing the order of nodes after successful searches can reduce the search cost for particular access patterns.

The project will begin with a basic singly linked list and then add two self-organizing strategies:

1. **Move-to-Front (MTF):** After a successful search, move the accessed node to the head of the list.
2. **Count-Based:** Track the number of successful accesses to each node and organize the list in descending order of access count.

The strategies will be compared with an ordinary list that does not reorganize itself. Using the same access sequences, and the number of nodes examined during searches will be recorded.

Insertion at the tail, deletion by value, pointer correctness, and the effect of maintaining a tail pointer and size counter will be demonstrated in the project.

## 2. Chosen Approach and Data Structures

### 2.1 Singly linked list

The core data structure will be a singly linked list implemented in `C` using dynamically allocated nodes.

The basic node contains:

- `data`: the integer stored in the node.
- `next`: a pointer to the next node.

The list will maintain a `head` pointer. We also plan to maintain a `tail` pointer and a `size` counter as extensions to the basic structure.

For the **Count-Based** strategy, each node will additionally store an access count. This field is not needed by the ordinary list or by Move-to-Front.

### 2.2 Planned structure

The initial design is conceptually represented as follows:

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

The `access_count` field is included to support the **Count-Based** strategy. The `tail` pointer and `size` counter will be maintained consistently during insertion, deletion, and reorganization.

integers are used as the stored values because they are straightforward to trace by hand and allow us to focus on linked-list behavior rather than data parsing or complex records.

### 2.3 Planned operations

The core implementation will include:

- Initialize an empty list.
- Insert a node at the head.
- Insert a node at the tail.
- Search for a value using sequential traversal.
- Delete a node by value.
- Display or traverse the list.
- Release all allocated nodes.
- Maintain the tail pointer and size counter correctly.
- Apply the selected self-organizing strategy after successful searches.

### 2.4 Why this approach fits the problem

With a linked list, its nodes can be reordered by changing pointers rather than shifting an array of elements. For example, **Move-to-Front** can move a node to the head by adjusting links.

However, a linked list does not provide constant-time indexed access. Searching by value still requires sequential traversal. Therefore, self-organization does not remove the worst-case `(O(n))` search bound; it may reduce the number of nodes examined for favorable access patterns.

With that, our group can investigate the trade-off between maintaining the list's order and improving access to frequently requested values.

## 3. Hand Trace: Move-to-Front

### 3.1 Initial list

The initial list is:

```text
Head
 |
 v
[A] -> [B] -> [C] -> [D] -> [E] -> [F] -> NULL
```

For this trace, we will use the access sequence:

```text
C, E, C, C
```

Each search starts at the head. The search cost measured by counting the nodes examined, including the node containing the target. After a successful search, the accessed node is moved to the head.

### 3.2 Trace table

| Step    | Accessed value | Nodes examined | List after the operation   |
| ------- | -------------- | -------------: | -------------------------- |
| Initial | —              |              — | A -> B -> C -> D -> E -> F |
| 1       | C              |              3 | C -> A -> B -> D -> E -> F |
| 2       | E              |              5 | E -> C -> A -> B -> D -> F |
| 3       | C              |              2 | C -> E -> A -> B -> D -> F |
| 4       | C              |              1 | C -> E -> A -> B -> D -> F |

Every list ends with `NULL`; it is omitted from the table to keep the rows compact.

### 3.3 Explanation of each step

**Step 1: Search for C**

The search examines A, then B, then C. The search cost is 3 node visits. Since C is not already at the head, it is detached from its current position and moved to the front.

```text
Before: A -> B -> C -> D -> E -> F -> NULL
After:  C -> A -> B -> D -> E -> F -> NULL
```

**Step 2: Search for E**

Starting from the new head, the search examines C, A, B, D, and E. The search cost is 5 node visits. E is then moved to the front.

```text
Before: C -> A -> B -> D -> E -> F -> NULL
After:  E -> C -> A -> B -> D -> F -> NULL
```

**Step 3: Search for C**

The search examines E and C, so the cost is 2 node visits. C is moved to the front.

```text
Before: E -> C -> A -> B -> D -> F -> NULL
After:  C -> E -> A -> B -> D -> F -> NULL
```

**Step 4: Search for C again**

C is already at the head, so only one node is examined. The list order does not change.

### 3.4 Comparison with an ordinary list

If the list remained in its original order throughout the same access sequence, the search costs would be:

| Access                             | Ordinary list | Move-to-Front |
| ---------------------------------- | ------------: | ------------: |
| C                                  |             3 |             3 |
| E                                  |             5 |             5 |
| C                                  |             3 |             2 |
| C                                  |             3 |             1 |
| **Total node visits**              |        **14** |        **11** |
| **Average node visits per search** |       **3.5** |      **2.75** |

With this sequence, **Move-to-Front** reduces the total number of node visits from 14 to 11.

This example does not prove that **Move-to-Front** is always faster. Different access sequences may produce different results, so a larger experiment is needed.

## 4. Complexity Expectations

The following table records our initial theoretical expectations. These will be checked against the implementation and discussed in the final report.

| Operation                      | Without a tail pointer                   | With a maintained tail pointer |
| ------------------------------ | ---------------------------------------- | ------------------------------ |
| Insert at head                 | (O(1))                                   | (O(1))                         |
| Insert at tail                 | (O(n))                                   | (O(1))                         |
| Search by value                | (O(n))                                   | (O(n))                         |
| Delete by value                | (O(n))                                   | (O(n))                         |
| Display the list               | (O(n))                                   | (O(n))                         |
| Read a maintained size counter | Not applicable unless a counter is added | (O(1))                         |

These bounds assume that the list is valid and that the tail pointer and size counter are maintained correctly.

A tail pointer makes insertion at the end constant-time because the program can access the last node directly. It does not make deletion by value constant-time: the program must still search for the value, and a singly linked list needs the predecessor to unlink the target.

For self-organizing strategies, a successful search requires sequential traversal before the node can be moved. Moving a node to the head then requires a constant number of pointer updates once its location and predecessor are known. The search itself can still take (O(n)) time in the worst case.

## 5. Expected Cost and Access Patterns

Two types of workload:

1. **Uniform or broadly distributed accesses:** values are requested with similar frequencies.
2. **Skewed accesses:** a small number of values are requested much more frequently than others.

For each workload, we will compare an ordinary list, **Move-to-Front**, and **Count-Based** using the same starting order and access sequence. Node visits are counted, because elapsed time can vary with computer, compiler, and other running processes.

The total node visits and average node visits per successful search will be reported. If unsuccessful searches are included, they will be recorded separately.

## 6. Proposed Count-Based Strategy

The **Count-Based** strategy is planned as an additional project feature.

Each node will store an access count, initially zero. Whenever a search successfully finds a node, its count will increase by one. The node will then be repositioned so that nodes with larger counts appear before nodes with smaller counts.

To make the behavior reproducible, ties will preserve the existing relative order of nodes with equal counts. This is the proposed tie-breaking rule and will be verified during implementation.

**Count-Based** and **Move-to-Front** will be compared using the same access sequences and starting order.

## 7. Test Plan

| Test | Setup or operation                            | Expected result                                                |
| ---- | --------------------------------------------- | -------------------------------------------------------------- |
| T1   | Initialize a list                             | Head and tail are `NULL`; size is zero                         |
| T2   | Insert at head into an empty list             | One node; head and tail point to it                            |
| T3   | Insert several values at the tail             | Values appear in insertion order                               |
| T4   | Search for the head value                     | Search succeeds after one node visit                           |
| T5   | Search for a middle value                     | Search returns the correct node                                |
| T6   | Search for a missing value                    | Search reports not found without invalid pointer access        |
| T7   | Delete the head by value                      | Head advances correctly; size decreases                        |
| T8   | Delete a middle value                         | Predecessor links to the deleted node's successor              |
| T9   | Delete the tail by value                      | Tail becomes the new final node; its `next` is `NULL`          |
| T10  | Delete the only node                          | Head and tail become `NULL`; size is zero                      |
| T11  | Delete a value not present                    | List remains unchanged                                         |
| T12  | Insert at tail with a maintained tail pointer | New node is appended and tail is updated                       |
| T13  | Perform repeated Move-to-Front searches       | Accessed nodes move to the head without losing nodes           |
| T14  | Perform repeated Count-Based searches         | Counts increase correctly and ordering follows the stated rule |
| T15  | Compare strategies on the same workload       | Node visits and average search costs are recorded consistently |
| T16  | Free the list                                 | All allocated nodes are released and list pointers are reset   |

### Required four-node defense scenario

Initial list:

```text
Head -> 10 -> 20 -> 30 -> 40 -> NULL
```

Insert `50` at the tail:

```text
Head -> 10 -> 20 -> 30 -> 40 -> 50 -> NULL
```

Delete `30` by value:

```text
Head -> 10 -> 20 -> 40 -> 50 -> NULL
```

The deletion requires finding `30`, then changing the `next` pointer of node `20` to point to node `40`. Since `30` was not the tail, the tail remains at `50`.

### Pointer correctness and memory safety

The tests will pay particular attention to the cases emphasized in Lesson 6:

- Empty lists and one-node lists.
- Updating the head when deleting the first node.
- Updating the predecessor's `next` pointer when deleting a middle node.
- Updating the tail when deleting the final node.
- Avoiding use-after-free, memory leaks, double-free errors, and invalid `NULL` dereferences.
- Ensuring that Move-to-Front and Count-Based do not lose nodes or create cycles.

We will compile with warnings enabled and inspect the list after operations during development.

## 8. Syllabus Mapping

The project's required syllabus mapping comes from Appendix A of the project rubric.

| Syllabus topic                             | Source                                                                   | How the project uses it                                                                                                           |
| ------------------------------------------ | ------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------- |
| Linked lists                               | L06                                                                      | Node structure, head pointer, insertion, deletion, sequential search, and pointer updates                                         |
| Linked-list implementation and testing     | L07                                                                      | Complete C implementation, test driver, edge cases, tracing, and debugging                                                        |
| Complexity analysis                        | L02                                                                      | Big-O notation, counting basic operations, and explaining how cost grows with input size                                          |
| List operations and sequential search cost | L06–L07                                                                  | Implement operations and count nodes examined by searches                                                                         |
| Expected/amortized cost                    | L02 for complexity foundations; further strategy analysis to be verified | Compare observed search costs under different workloads and distinguish empirical averages, expected cost, and amortized analysis |

## 9. Initial AI Brainstorming Log

The following entries summarize the initial project-planning questions discussed while interpreting the rubric and course materials.

| Initial prompt or question                                                                                                           | Purpose                                      | Decision or outcome                                                                                       |
| ------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| What project can be designed for the Self-Organizing Lists family when the rubric provides no detailed implementation specification? | Establish a feasible project scope           | Use a singly linked list with integer values and self-organizing search strategies                        |
| Does the proposed plan satisfy Component C, Syllabus Alignment?                                                                      | Check required concept coverage              | Explicitly map list operations, sequential search cost, and expected/amortized cost to the project        |
| Can the implementation be based on the supplied course PDFs?                                                                         | Avoid silently introducing untaught material | Use L02, L06, and L07 as primary references; flag unconfirmed strategies as extensions                    |
| What should the Week 1 hand trace demonstrate?                                                                                       | Meet the planning evidence requirement       | Trace six nodes under Move-to-Front and prepare the four-node insert/delete scenario for defense practice |

AI assistance was used for brainstorming, organizing the plan, and identifying concepts that need verification. The implementation, test results, and final complexity claims will need to be checked against the actual code and the course materials. No implementation or experiment results are claimed in this Week 1 report.

## 10. References

1. **Lesson 2 — Overview: From Problem to Program, Data Types, Algorithm Complexity.** Supplied course PDF, especially the sections on pointers, dynamic memory, operation counting, and Big-O notation.
2. **Lesson 6 — Lists Continued: Linked Lists, Singly Linked List Operations.** Supplied course PDF, especially the sections on node structures, insertion, deletion, sequential search, pointer correctness, and complexity comparisons.
3. **Lesson 7 — Practice Lab: Building and Using Singly Linked Lists.** Supplied course PDF, especially the implementation checklist, test driver, edge-case testing, and debugging guidance.
4. **Project rubric — Appendix A and Appendix B.** Self-Organizing Lists syllabus mapping and LIST-family concept questions and live-modification tasks.

## 11. Week 1 Deliverables and Next Steps

At the end of Week 1, the planned evidence is:

- [x] Restate the problem in our own words.
- [x] Choose a data structure and justify the approach.
- [x] Provide a hand trace with six elements.
- [x] Prepare a test plan covering normal cases and edge cases.
- [x] Map the project to the syllabus topics.
- [x] Record initial brainstorming prompts.
- [ ] Implement and verify the basic linked-list operations in Week 2.
- [ ] Validate the hand trace against the implementation.
- [ ] Implement and compare the self-organizing strategies.
- [ ] Record experimental data and finalize the complexity analysis.

**Week 1 conclusion:** The project will use a singly linked list implemented from scratch in `C`. The foundational operations and pointer management are grounded in Lessons 6 and 7, while the complexity analysis is grounded in Lesson 2. Self-organizing strategies will be treated as explicit project extensions, tested experimentally, and explained rather than assumed to be part of the taught syllabus.
