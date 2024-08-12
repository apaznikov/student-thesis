#ifndef BINBOOST_BB_H
#define BINBOOST_BB_H

#ifdef __cplusplus
extern "C" {
#endif

void bb_jit_begin(void);
void bb_jit_end(void);

#ifdef BB_DISABLED

#define BB_JIT_BEGIN() \
  do {                 \
  } while (false)

#define BB_JIT_END() \
  do {               \
  } while (false)

#else

#define BB_JIT_BEGIN() bb_jit_begin()
#define BB_JIT_END() bb_jit_end()

#endif  // BB_DISABLED

#ifdef __cplusplus
}
#endif

#endif  // !BINBOOST_BINBOOST_H
