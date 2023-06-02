#include <math.h>
#include <omp.h>
#define ceild(n, d) ceil(((double)(n)) / ((double)(d)))
#define floord(n, d) floor(((double)(n)) / ((double)(d)))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define M 3199
#define N 3199
#define K 3199
#define alpha 1
#define beta 1

#pragma declarations
double A[M][K];
double B[K][N];
double C[M][N];
#pragma enddeclarations

#ifdef PERFCTR
#include <papi.h>

#include "papiStdEventDefs.h"
#include "papi_defs.h"
#endif

#ifdef TIME
#define IF_TIME(foo) foo;
#else
#define IF_TIME(foo)
#endif

void init_array() {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      A[i][j] = (i + j);
      B[i][j] = (double)(i * j);
      C[i][j] = 0.0;
    }
  }
}

void print_array() {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      fprintf(stderr, "%lf ", C[i][j]);
      if (j % 80 == 79) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
  }
}

double rtclock() {
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday(&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d", stat);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}
double t_start, t_end;

int main() {
  int i, j, k;

  init_array();

#ifdef PERFCTR
  PERF_INIT;
#endif

  IF_TIME(t_start = rtclock());

  /* Copyright (C) 1991-2022 Free Software Foundation, Inc.
     This file is part of the GNU C Library.

     The GNU C Library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     The GNU C Library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with the GNU C Library; if not, see
     <https://www.gnu.org/licenses/>.  */
  /* This header is separate from features.h so that the compiler can
     include it implicitly at the start of every compilation.  It must
     not itself include <features.h> or any other header that includes
     <features.h> because the implicit include comes before any feature
     test macros that may be defined in a source file before it first
     explicitly includes a system header.  GCC knows the name of this
     header in order to preinclude it.  */
  /* glibc's intent is to support the IEC 559 math functionality, real
     and complex.  If the GCC (4.9 and later) predefined macros
     specifying compiler intent are available, use them to determine
     whether the overall intent is to support these features; otherwise,
     presume an older compiler has intent to support these features and
     define these macros by default.  */
  /* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
     synchronized with ISO/IEC 10646:2017, fifth edition, plus
     the following additions from Amendment 1 to the fifth edition:
     - 56 emoji characters
     - 285 hentaigana
     - 3 additional Zanabazar Square characters */
  int t1, t2, t3, t4, t5, t6, t7, t8, t9;
  int lb, ub, lbp, ubp, lb2, ub2;
  register int lbv, ubv;
  /* Start of CLooG code */
  if ((K >= 1) && (M >= 1) && (N >= 1)) {
    lbp = 0;
    ubp = floord(M - 1, 128);
#pragma omp parallel for private(lbv, ubv, t2, t3, t4, t5, t6, t7, t8, t9)
    for (t1 = lbp; t1 <= ubp; t1++) {
      for (t2 = 0; t2 <= floord(N - 1, 256); t2++) {
        for (t3 = 0; t3 <= floord(K - 1, 128); t3++) {
          for (t4 = 16 * t1; t4 <= min(floord(M - 1, 8), 16 * t1 + 15); t4++) {
            for (t5 = 2 * t2; t5 <= min(floord(N - 1, 128), 2 * t2 + 1); t5++) {
              for (t6 = 16 * t3; t6 <= min(floord(K - 1, 8), 16 * t3 + 15);
                   t6++) {
                for (t7 = 8 * t4; t7 <= min(M - 1, 8 * t4 + 7); t7++) {
                  for (t8 = 8 * t6; t8 <= min(K - 1, 8 * t6 + 7); t8++) {
                    lbv = 128 * t5;
                    ubv = min(N - 1, 128 * t5 + 127);
#pragma ivdep
#pragma vector always
                    for (t9 = lbv; t9 <= ubv; t9++) {
                      C[t7][t9] =
                          beta * C[t7][t9] + alpha * A[t7][t8] * B[t8][t9];
                      ;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  /* End of CLooG code */

  IF_TIME(t_end = rtclock());
  IF_TIME(fprintf(stdout, "%0.6lfs\n", t_end - t_start));

#ifdef PERFCTR
  PERF_EXIT;
#endif

  if (fopen(".test", "r")) {
#ifdef MPI
    if (my_rank == 0) {
      print_array();
    }
#else
    print_array();
#endif
  }

  return 0;
}
