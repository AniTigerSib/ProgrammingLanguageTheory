## LAB_1

#### Task

- Construct a deterministic finite state machine of the multiline comment recognizer of the C language and implement it programmatically.
- Improve the recognizer for correct removal of C (multiline) and C++ (single-line) language comments.

#### File structure

- `img` : *directory*, contains screenshots of finite state machines, required by the task;
- `test_data` : *directory* contains data (source .c files) for application testing, required by the task;
- `task_1.c` : *source file* represents the solution to task 1;
- `task_2.c` : *source file* represents the solution to task 2;
- `main.c` : *source file* similar to the `task_2.c`.

#### Before building

To build source file, representing the solution to task 1 - replace name of source file in Makefile variable `SOURCES` to `task_1.c`. Similarly for task 2.

#### Building sources

```
mkdir build
mkdir test_result
build
```