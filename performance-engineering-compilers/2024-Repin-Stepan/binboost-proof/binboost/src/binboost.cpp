#include "arch/intrinsics.hpp"
#include "binboost/bb.h"

#include <cstdio>
#include <cstdlib>

#include "arch/intrinsics.hpp"
#include "basic_block_parser.hpp"
#include "code_aligner.hpp"
#include "code_analyzer.hpp"

extern "C" {

std::uintptr_t bb_jit_begin_impl(std::uintptr_t opt_code_start) {
  using namespace bb;

  if (opt_code_start == 0) {
    // Случай только для x86.
    //
    // @todo Сделать условную компиляцию для таких вещей.
    opt_code_start = bb::arch::GetReturnAddr();
  }

  std::printf("Parsing binary code from 0x%lx...\n", opt_code_start);

  BasicBlockParser parser(opt_code_start);
  auto bbs = parser.Parse();
  if (!bbs.IsValid()) {
    std::fprintf(stderr,
                 "Panic: failed to find bb_jit_end() call (from=0x%02lx)\n",
                 opt_code_start);
    std::exit(1);
  }

  //CodeAligner aligner(&bbs);
  //aligner.DoAlign();

  bbs.Dump(stdout);

  // @todo generator
  CodeAnalyzer analyzer(&bbs);
  std::printf("Copying code...\n");
  auto new_code_start = analyzer.CopyCode();

  std::printf("new_code_start=0x%lx\n", new_code_start);

  return new_code_start;
}

void bb_jit_end() {
  // Чтобы компилятор не соптимизировал функцию.
  __asm__ volatile("");
}

#ifdef BB_EXCLUDE_TRAMPOLINE

void bb_jit_begin() {
  fprintf(
      stderr,
      "Panic: BB_EXCLUDE_TRAMPOLINE enabled, program launch is imposible\n");
  std::exit(1);
}

#endif  // BB_EXCLUDE_TRAMPLINE
}
