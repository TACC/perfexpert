--
-- Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
--
-- $COPYRIGHT$
--
-- Additional copyrights may follow
--
-- This file is part of PerfExpert.
--
-- PerfExpert is free software: you can redistribute it and/or modify it under
-- the terms of the The University of Texas at Austin Research License
--
-- PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
-- A PARTICULAR PURPOSE.
--
-- Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
--
-- $HEADER$
--

--
-- Enable foreign keys
--
PRAGMA foreign_keys = ON;

--
-- Create tables
--

CREATE TABLE IF NOT EXISTS perfexpert_experiment (
    perfexpert_id INTEGER PRIMARY KEY,
    command VARCHAR,
    mpi_tasks INTEGER,
    threads INTEGER
);

CREATE TABLE IF NOT EXISTS category (
    id    INTEGER NOT NULL,
    name  VARCHAR NOT NULL,
    short VARCHAR NOT NULL,
    PRIMARY KEY (id)
);

--
-- depth field means:
--     0 = functions
--     1 = AutoSCOPE's 'loop1'
--     2 = AutoSCOPE's 'loop2'
--     3 = AutoSCOPE's 'loop3'
--
CREATE TABLE IF NOT EXISTS recommendation (
    id      INTEGER PRIMARY KEY,
    name    VARCHAR NOT NULL,
    reason  VARCHAR NOT NULL,
    example VARCHAR,
    depth   INTEGER
);

CREATE TABLE IF NOT EXISTS language (
    id    INTEGER PRIMARY KEY,
    name  VARCHAR NOT NULL
);

CREATE TABLE IF NOT EXISTS pattern (
    id         INTEGER PRIMARY KEY,
    language   INTEGER REFERENCES language,
    recognizer VARCHAR NOT NULL,
    name       VARCHAR NOT NULL
);

CREATE TABLE IF NOT EXISTS transformation (
    id          INTEGER PRIMARY KEY,
    language    INTEGER REFERENCES language,
    transformer VARCHAR NOT NULL,
    name        VARCHAR NOT NULL
);

CREATE TABLE IF NOT EXISTS architecture (
    id    INTEGER PRIMARY KEY,
    name  VARCHAR,
    short VARCHAR
);

CREATE TABLE IF NOT EXISTS strategy (
    id        INTEGER PRIMARY KEY,
    name      VARCHAR NOT NULL,
    statement VARCHAR NOT NULL
);

CREATE TABLE IF NOT EXISTS ra (
    rid INTEGER REFERENCES recommendation,
    aid INTEGER REFERENCES architecture
);

CREATE TABLE IF NOT EXISTS rt (
    rid INTEGER REFERENCES recommendation,
    tid INTEGER REFERENCES transformation
);

CREATE TABLE IF NOT EXISTS tp (
    tid INTEGER REFERENCES transformation,
    pid INTEGER REFERENCES pattern(id)
);

CREATE TABLE IF NOT EXISTS rc (
    rid    INTEGER REFERENCES recommendation,
    cid    INTEGER REFERENCES category,
    weight REAL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS rf (
    rid    INTEGER REFERENCES recommendation,
    fid    INTEGER REFERENCES function,
    weight REAL
);

CREATE TABLE IF NOT EXISTS perfexpert_hotspot (
    perfexpert_id INTEGER NOT NULL,
    id            INTEGER PRIMARY KEY,
    name          VARCHAR NOT NULL,
    type          INTEGER NOT NULL,
    profile       VARCHAR NOT NULL,
    module        VARCHAR NOT NULL,
    file          VARCHAR NOT NULL,
    line          INTEGER NOT NULL,
    depth         INTEGER NOT NULL,
    relevance     INTEGER
);

CREATE TABLE IF NOT EXISTS perfexpert_event (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    name          VARCHAR NOT NULL,
    thread_id     INTEGER NOT NULL,
    mpi_task      INTEGER NOT NULL,
    experiment    INTEGER NOT NULL,
    value         REAL    NOT NULL,
    hotspot_id    INTEGER NOT NULL,
        
    FOREIGN KEY (hotspot_id) REFERENCES perfexpert_hotspot(id)
);

--
-- Populate tables
--
BEGIN TRANSACTION;

INSERT INTO language (id, name) VALUES (1, 'C89');
INSERT INTO language (id, name) VALUES (2, 'C');
INSERT INTO language (id, name) VALUES (3, 'Ada83');
INSERT INTO language (id, name) VALUES (4, 'C_plus_plus');
INSERT INTO language (id, name) VALUES (5, 'Cobol74');
INSERT INTO language (id, name) VALUES (6, 'Cobol85');
INSERT INTO language (id, name) VALUES (7, 'Fortran77');
INSERT INTO language (id, name) VALUES (8, 'Fortran90');
INSERT INTO language (id, name) VALUES (9, 'Pascal83');
INSERT INTO language (id, name) VALUES (10, 'Modula2');
INSERT INTO language (id, name) VALUES (11, 'Java');
INSERT INTO language (id, name) VALUES (12, 'C99');
INSERT INTO language (id, name) VALUES (13, 'Ada95');
INSERT INTO language (id, name) VALUES (14, 'Fortran95');
INSERT INTO language (id, name) VALUES (15, 'PLI');
INSERT INTO language (id, name) VALUES (16, 'ObjC');
INSERT INTO language (id, name) VALUES (17, 'ObjC_plus_plus');
INSERT INTO language (id, name) VALUES (18, 'UPC');
INSERT INTO language (id, name) VALUES (19, 'D');
INSERT INTO language (id, name) VALUES (20, 'Python');
INSERT INTO language (id, name) VALUES (21, 'OpenCL');
INSERT INTO language (id, name) VALUES (22, 'Go');
INSERT INTO language (id, name) VALUES (32768, 'lo_user');
INSERT INTO language (id, name) VALUES (32769, 'Mips_Assembler');
INSERT INTO language (id, name) VALUES (34661, 'Upc');
INSERT INTO language (id, name) VALUES (37121, 'ALTIUM_Assembler');
INSERT INTO language (id, name) VALUES (36865, 'SUN_Assembler');
INSERT INTO language (id, name) VALUES (65535, 'hi_user');

INSERT INTO architecture (id, short, name) VALUES
    (1, 'x86_64', '64 bit extension of the x86 instruction set');

INSERT INTO category (id, short, name) VALUES
    (1, 'd-l1', 'data_accesses.L1d_hits');
INSERT INTO category (id, short, name) VALUES
    (2, 'd-l2', 'data_accesses.L2d_hits');
INSERT INTO category (id, short, name) VALUES
    (3, 'd-mem', 'data_accesses.LLC_misses');
INSERT INTO category (id, short, name) VALUES
    (4, 'd-tlb', 'data_TLB.overall');
INSERT INTO category (id, short, name) VALUES
    (5, 'i-access', 'instruction_accesses.overall');
INSERT INTO category (id, short, name) VALUES
    (6, 'i-tlb', 'instruction_TLB.overall');
INSERT INTO category (id, short, name) VALUES
    (7, 'br-i', 'branch_instructions.overall');
INSERT INTO category (id, short, name) VALUES
    (8, 'fpt-fast', 'point_instr.fast_FP_instr');
INSERT INTO category (id, short, name) VALUES
    (9, 'fpt-slow', 'point_instr.slow_FP_instr');

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (1, 'eliminate common subexpressions involving memory accesses',
    'this optimization reduces the number of executed (slow) memory accesses',
    'd[i] = a * b[i] + c[i];
y[i] = a * b[i] + x[i];
 =====>
temp = a * b[i];
d[i] = temp + c[i];
y[i] = temp + x[i];', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (2, 'eliminate floating-point operations through distributivity',
    'this optimization reduces the number of executed floating-point operations',
    'd]i] = a[i] * b[i] + a[i] * c[i];
 =====>
d[i] = a[i] * (b[i] + c[i]);', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (3, 'eliminate floating-point operations through associativity',
    'this optimization reduces the number of executed floating-point operations',
    'd[i] = a[i] * b[i] * c[i];
y[i] = x[i] * a[i] * b[i];
 =====>
temp = a[i] * b[i];
d[i] = temp * c[i];
y[i] = x[i] * temp;', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (4, 'move loop invariant computations out of loop',
    'this optimization reduces the number of executed floating-point operations',
    'loop i {
  x = x + a * b * c[i];
}
 =====>
temp = a * b;
loop i {
  x = x + temp * c[i];
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (5, 'use float instead of double data type if loss of precision is acceptable',
    'this optimization reduces the amount of memory required to store floating-point
data, which often makes accessing the data faster',
'double a[n];
 =====>
float a[n];', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (6, 'compare squared values instead of computing the square root',
    'this optimization replaces a slow operation with equivalent but much faster
operations',
    'if (x < sqrt(y)) {...}
 =====>
if ((x < 0.0) || (x*x < y)) {...}', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (7, 'compute the reciprocal outside of loop and use multiplication inside the loop',
    'this optimization replaces many slow operations with one slow and many fast
operations that accomplish the same',
    'loop i {
  a[i] = b[i] / c;
}
 =====>
cinv = 1.0 / c;
loop i {
  a[i] = b[i] * cinv;
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (8, 'accumulate and then normalize instead of normalizing each element',
    'this optimization replaces many slow operations with a single slow operation
that accomplishes the same',
    'loop i {
  x = x + a[i] / b;
}
 =====>
loop i {
  x = x + a[i];
}
x = x / b;', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (9, 'use trivial assignments inside THEN/ELSE to allow the use of conditional moves',
    'this optimization may allow the compiler to replace a code sequence with an
equivalent but faster code sequence that uses no branches',
    'if (x < y)
  a = x + y;
 =====>
temp = x + y;
if (x < y)
  a = temp;', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (10, 'express Boolean logic in form of integer computation',
    'this optimization replaces a code sequence with an equivalent code sequence that
is faster because fewer branches are executed',
    'if ((a == 0) && (b == 0) && (c == 0)) {...}
 =====>
if ((a | b | c) == 0) {...}', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (11, 'remove IF',
    'this optimization replaces a code sequence with an equivalent code sequence that
may be faster because no branches are needed',
    '/* x is 0 or -1 */
if (x == 0)
  a = b;
else
  a = c;
 =====>
a = (b & ~x) | (c & x);', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (12, 'unroll loops (more)',
    'this optimization replaces a code sequence with an equivalent code sequence that
is faster because fewer branches are executed',
    'loop i {
  a[i] = a[i] * b[i];
}
 =====>
loop i step 2 {
  a[i] = a[i] * b[i];
  a[i+1] = a[i+1] * b[i+1];
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (13, 'move a loop around a subroutine call into the subroutine',
    'this optimization replaces a code sequence with an equivalent code sequence that
is faster because fewer calls are executed',
    'f(x) {...x...};
loop i {
  f(a[i]);
}
 =====>
f(x[]) {
  loop j {
    ...x[j]...
  }
};
f(a);', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (14, 'move loop invariant tests out of loop',
    'this optimization replaces a code sequence with an equivalent code sequence that
is faster because fewer branches are executed',
    'loop i {
  if (x < y)
    a[i] = x * b[i];
  else
    a[i] = y * b[i];
}
 =====>
if (x < y) {
  loop i {
    a[i] = x * b[i];
  }
} else {
  loop i {
    a[i] = y * b[i];
  }
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (15, 'special-case the most often used loop count(s)',
    'this optimization replaces a code sequence with an equivalent sequence that is
faster because fixed loop boundaries often enable better compiler optimizations',
    'for (i = 0; i < n; i++) {...}
 =====>
if (n == 4) {
  for (i = 0; i < 4; i++) {...}
} else {
  for (i = 0; i < n; i++) {...}
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (16, 'use inlining',
    'this optimization replaces a code sequence with an equivalent code sequence that
is faster because fewer control transfers are executed',
    'float f(float x) {
  return x * x;
}
z = f(y);
 =====>
z = y * y;', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (17, 'change the order of subroutine calls',
    'this optimization, when allowed, may yield faster execution when it results in
more opportunity for compiler optimizations',
    'f(); h();
 =====>
h(); f();', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (18, 'factor out sequences of common code into subroutines',
    'this optimization reduces the code size, which may improve the instruction cache
performance',
    'same_code;
same_code;
 =====>
void f() {same_code;}
f();
f();', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (19, 'sort subroutines by call chains (subroutine coloring)',
    'this optimization moves functions to potentially better starting addresses in
memory',
    'f() {...}
g() {...}
h() {...}
loop {
  f();
  h();
}
 =====>
g() {...}
f() {...}
h() {...}
loop {
  f();
  h();
}', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (20, 'lower the loop unroll factor',
    'this optimization reduces the code size, which may improve the instruction cache
performance',
    'loop i step 4 {
  code_i;
  code_i+1;
  code_i+2;
  code_i+3;
}
 =====>
loop i step 2 {
  code_i;
  code_i+1;
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (21, 'make subroutines more general and use them more',
    'this optimization reduces the code size, which may improve the instruction cache
performance',
    'void f() {
  statements1;
  statementsX;
}
void g() {
  statements2;
  statementsX;
}
 =====>
void fg(int flag) {
  if (flag) {
    statements1;
  } else {
    statements2;
  }
  statementsX;
}', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (22, 'split off cold code into separate subroutines and place them at end of file',
    'this optimization separates rarely from frequently used code, which may improve
the instruction cache performance',
    'if (unlikely_condition) {
  lots_of_code
}
 =====>
void f() {lots_of_code}
...
if (unlikely_condition)
  f();', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (23, 'use trace scheduling to reduce the branch taken frequency',
    'this optimization replaces a code sequence with an equivalent code sequence that
may be faster because it reduces (taken) branches and may enable better compiler
optimizations',
    'if (likely_condition)
  f();
else
  g();
h();
 =====>
if (!likely_condition) {
  g(); h();
} else {
  f(); h();
}', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (24, 'move loop invariant memory accesses out of loop',
    'this optimization reduces the number of executed (slow) memory accesses',
    'loop i {
  a[i] = b[i] * c[j]
}
 =====>
temp = c[j];
loop i {
  a[i] = b[i] * temp;
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (25, 'compute values rather than loading them if doable with few operations',
    'this optimization replaces (slow) memory accesses with equivalent but faster
computations',
    'loop i {
  t[i] = a[i] * 0.5;
}
loop i {
  a[i] = c[i] - t[i];
}
 =====>
loop i {
  a[i] = c[i] - (a[i] * 0.5);
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (26, 'enable the use of vector instructions to transfer more data per access',
    'this optimization increases the memory bandwidth',
    'align arrays, use only stride-one accesses, make loop count
even (pad arrays):
struct {
  double a, b;
} s[127];
for (i = 0; i < 127; i++) {
  s[i].a = 0;
  s[i].b = 0;
}
 =====>
__declspec(align(16)) double a[128], b[128];
for (i = 0; i < 128; i++) {
  a[i] = 0;
  b[i] = 0;
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (27, 'copy data into local scalar variables and operate on the local copies',
    'this optimization replaces (slow) memory accesses with equivalent but faster
computations',
    'x = a[i] * a[i];
...
a[i] = x / b;
...
b = a[i] + 1.0;
 =====>
t = a[i];
x = t * t;
...
a[i] = t = x / b;
...
b = t + 1.0;', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (28, 'align data, especially arrays and structs',
    'this optimization enables the use of vector instructions, which increase the
memory bandwidth',
    'int x[1024];
 =====>
__declspec(align(16)) int x[1024];', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (29, 'help the compiler by marking pointers to non-overlapping data with "restrict"',
    'this optimization, when applicable, enables the compiler to tune the code more
aggressively',
    'void *a, *b;
 =====>
void * restrict a, * restrict b;', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (30, 'fuse multiple loops that access the same data',
    'this optimization enables the reuse of loaded data',
    'loop i {
  a[i] = x[i];
}
loop i {
  b[i] = x[i] - 1;
}
 =====>
loop i {
  a[i] = x[i];
  b[i] = x[i] - 1;
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (31, 'change the order of loops',
    'this optimization may improve the memory access pattern and make it more cache
and TLB friendly',
    'loop i {
  loop j {...}
}
 =====>
loop j {
  loop i {...}
}', 2);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (32, 'employ loop blocking and interchange',
    'this optimization may improve the memory access pattern and make it more cache
and TLB friendly, choose s such that ((S^2)+(2*s)) is less than the cache size',
    'loop i {
  loop k {
    loop j {
      c[i][j] = c[i][j] + a[i][k] * b[k][j];
    }
  }
}
 =====>
loop k step s {
  loop j step s {
    loop i {
      for (kk = k; kk < k + s; kk++) {
        for (jj = j; jj < j + s; jj++) {
          c[i][jj] = c[i][jj] + a[i][kk] * b[kk][jj];
        }
      }
    }
  }
}', 3);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (33, 'unroll outer loop',
    'this optimization may reduce the number of (slow) memory accesses',
    'loop i {
  loop j {
    a[i][j] = b[i][j] * c[j];
  }
}
 =====>
loop i step 4 {
  loop j {
    a[i][j] = b[i][j] * c[j];
    a[i+1][j] = b[i+1][j] * c[j];
    a[i+2][j] = b[i+2][j] * c[j];
    a[i+3][j] = b[i+3][j] * c[j];
  }
}', 2);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (34, 'access arrays directly instead of using local copies',
    'this optimization reduces the memory footprint, which may improve cache
performance',
    'loop j {
  a[j] = b[i][j][k];
}
...
loop j {
  ... a[j] ...
}
 =====>
loop j {
  ... b[i][j][k] ...
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (35, 'reuse temporary arrays for different operations',
    'this optimization reduces the memory footprint, which may improve cache
performance',
    'loop i {
  t1[i] = ...;
  ... t1[i] ...;
}
...
loop j {
  t2[j] = ...;
  ... t2[j] ...;
}
 =====>
loop i {
  t[i] = ...;
  ... t[i] ...;
}
...
loop j {
  t[j] = ...;
  ... t[j] ...;
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (36, 'avoid unnecessary array updates',
    'this optimization reduces the number of memory writes',
    'loop i {
  a[i] = ...;
  ... a[i] ...
}
// array a[] not read
 =====>
loop i {
  temp = ...;
  ... temp ...
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (37, 'use smaller types (e.g., float instead of double or short instead of int)',
    'this optimization reduces the memory footprint, which may improve cache
performance',
    'double a[n];
 =====>
float a[n];', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (38, 'apply loop fission so every loop accesses just a couple of different arrays',
    'this optimization reduces the number of active memory pages which may improve
DRAM performance',
    'loop i {
  a[i] = a[i] * b[i] - c[i];
}
 =====>
loop i {
  a[i] = a[i] * b[i];
}
loop i {
  a[i] = a[i] - c[i];
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (39, 'componentize important loops by factoring them into their own subroutines',
    'this optimization may allow the compiler to optimize the loop independently and
thus tune it better',
    'loop i {...}
...
loop j {...}
 =====>
void li() {loop i {...}}
void lj() {loop j {...}}
...
li();
...
lj();', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (40, 'split structs into hot and cold parts, where hot part has pointer to cold part',
    'this optimization separates rarely and frequently used data which may improve
cache performance',
    'struct s {
  hot_field;
  many_cold_fields;
} a[n];
 =====>
struct s_hot {
  hot_field;
  struct s_cold *ptr;
} a_hot[n];
struct s_cold {
  many_cold_fields;
} a_cold[n];', 0);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (41, 'pad memory areas so that temporal elements do not map to same set in cache',
    'this optimization reduces the chance of data overlap in the caches, which may
improve cache performance, the final size of each array should be an integer
multiple of the cache line size (typically 64 bytes) but should not be a small
integer multiple of the cache size',
    'double a[const * cache_size/8], b[const * cache_size/8];
loop i {
  ... a[i] + b[i] ...
}
 =====>
double a[const * cache_size/8 + 8], b[const * cache_size/8 + 8];
loop i {
  ... a[i] + b[i] ...
}', 1);

INSERT INTO recommendation (id, name, reason, example, depth) VALUES
    (42, 'allocate an array of elements instead of each element individually',
    'this optimization reduces the memory footprint and enhances spatial locality,
which may improve cache performance',
    'loop {
  ... c = malloc(1); ...
}
 =====>
top = n;
loop {
  if (top == n) {
    tmp = malloc(n);
    top = 0;
  }
  ...
  c = &tmp[top++]; ...
}', 1);

INSERT INTO rc (rid, cid) VALUES (1, 1);
INSERT INTO rc (rid, cid) VALUES (2, 8);
INSERT INTO rc (rid, cid) VALUES (2, 9);
INSERT INTO rc (rid, cid) VALUES (3, 8);
INSERT INTO rc (rid, cid) VALUES (3, 9);
INSERT INTO rc (rid, cid) VALUES (4, 8);
INSERT INTO rc (rid, cid) VALUES (4, 9);
INSERT INTO rc (rid, cid) VALUES (5, 1);
INSERT INTO rc (rid, cid) VALUES (5, 9);
INSERT INTO rc (rid, cid) VALUES (6, 9);
INSERT INTO rc (rid, cid) VALUES (7, 9);
INSERT INTO rc (rid, cid) VALUES (8, 9);
INSERT INTO rc (rid, cid) VALUES (9, 7);
INSERT INTO rc (rid, cid) VALUES (10, 7);
INSERT INTO rc (rid, cid) VALUES (11, 7);
INSERT INTO rc (rid, cid) VALUES (12, 7);
INSERT INTO rc (rid, cid) VALUES (13, 7);
INSERT INTO rc (rid, cid) VALUES (14, 7);
INSERT INTO rc (rid, cid) VALUES (15, 7);
INSERT INTO rc (rid, cid) VALUES (16, 6);
INSERT INTO rc (rid, cid) VALUES (17, 6);
INSERT INTO rc (rid, cid) VALUES (18, 5);
INSERT INTO rc (rid, cid) VALUES (19, 5);
INSERT INTO rc (rid, cid) VALUES (19, 6);
INSERT INTO rc (rid, cid) VALUES (20, 5);
INSERT INTO rc (rid, cid) VALUES (21, 5);
INSERT INTO rc (rid, cid) VALUES (22, 5);
INSERT INTO rc (rid, cid) VALUES (23, 5);
INSERT INTO rc (rid, cid) VALUES (24, 1);
INSERT INTO rc (rid, cid) VALUES (25, 2);
INSERT INTO rc (rid, cid) VALUES (25, 3);
INSERT INTO rc (rid, cid) VALUES (26, 1);
INSERT INTO rc (rid, cid) VALUES (27, 1);
INSERT INTO rc (rid, cid) VALUES (28, 1);
INSERT INTO rc (rid, cid) VALUES (29, 1);
INSERT INTO rc (rid, cid) VALUES (30, 2);
INSERT INTO rc (rid, cid) VALUES (30, 3);
INSERT INTO rc (rid, cid) VALUES (31, 3);
INSERT INTO rc (rid, cid) VALUES (31, 4);
INSERT INTO rc (rid, cid) VALUES (32, 3);
INSERT INTO rc (rid, cid) VALUES (33, 1);
INSERT INTO rc (rid, cid) VALUES (34, 2);
INSERT INTO rc (rid, cid) VALUES (34, 3);
INSERT INTO rc (rid, cid) VALUES (35, 2);
INSERT INTO rc (rid, cid) VALUES (35, 3);
INSERT INTO rc (rid, cid) VALUES (36, 2);
INSERT INTO rc (rid, cid) VALUES (36, 3);
INSERT INTO rc (rid, cid) VALUES (37, 3);
INSERT INTO rc (rid, cid) VALUES (37, 4);
INSERT INTO rc (rid, cid) VALUES (38, 3);
INSERT INTO rc (rid, cid) VALUES (39, 1);
INSERT INTO rc (rid, cid) VALUES (39, 2);
INSERT INTO rc (rid, cid) VALUES (39, 3);
INSERT INTO rc (rid, cid) VALUES (40, 3);
INSERT INTO rc (rid, cid) VALUES (41, 3);
INSERT INTO rc (rid, cid) VALUES (42, 3);

INSERT INTO pattern (id, language, recognizer, name) VALUES
    (1, 1, 'nested_c_loops', 'recognize two or more perfect nested loops');

INSERT INTO transformation (id, language, transformer, name) VALUES
    (1, 1, 'loop_interchange', 'interchange two perfect nested loops');
INSERT INTO transformation (id, language, transformer, name) VALUES
    (2, 1, 'loop_tiling', 'tile two perfect nested loops');

INSERT INTO rt (rid, tid) VALUES (31, 1);
INSERT INTO rt (rid, tid) VALUES (32, 1);

INSERT INTO tp (tid, pid) VALUES (1, 1);
INSERT INTO tp (tid, pid) VALUES (2, 1);

--
-- AustoSCOPE recommendation selection algorithm
--
-- @HID is replaced by the hotspot ID
--
INSERT INTO strategy (name, statement) VALUES
    ('AutoSCOPE recommendation algorithm',
    "SELECT
    recommendation.id,
    SUM(
        lcpi_metric.value - (maximum * 0.1)
    ) AS score
FROM
    recommendation
INNER JOIN rc ON recommendation.id = rc.rid
INNER JOIN category ON category.id = rc.cid
JOIN lcpi_metric ON category.name = lcpi_metric.name
JOIN (
    SELECT
        MAX(value) AS maximum
    FROM
        lcpi_metric
    WHERE
        hotspot_id = @HID
    AND (
        SELECT
            (
                MAX(
                    CASE name
                    WHEN 'overall' THEN
                        (value * 100)
                    END
                ) / (
                    (
                        0.5 * (
                            100 - MAX(
                                CASE name
                                WHEN 'ratio.floating_point' THEN
                                    value
                                END
                            )
                        )
                    ) + (
                        1.0 * MAX(
                            CASE name
                            WHEN 'ratio.floating_point' THEN
                                value
                            END
                        )
                    )
                )
            )
        FROM
            lcpi_metric
        WHERE
            hotspot_id = @HID
    ) > 1
)
JOIN hotspot ON hotspot.id = lcpi_metric.hotspot_id
WHERE
    (
        recommendation.depth <= hotspot.depth
    )
AND lcpi_metric.hotspot_id = @HID
GROUP BY
    recommendation.id
ORDER BY
    score DESC;");

END TRANSACTION;
