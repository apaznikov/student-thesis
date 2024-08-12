#pragma once

#include <cstdio>
#include "arch/instr.hpp"
#include "arch/intrinsics.hpp"

#include "basic_block_parser.hpp"

namespace bb {

class CodeAligner {
 public:
  explicit CodeAligner(BasicBlockCollection* bb_col) : bb_col_(bb_col) {}

  BasicBlock* FindDominator() {
    //std::vector<bool> visited(bb_col_->Size(), false);
    //std::vector<bool> rec_stack(bb_col_->Size(), false);

    return nullptr;
  }

  void DoAlign() {
    // @todo hehe, find dominator
    auto* target_bb = &bb_col_->GetFirst();
    auto start_addr = arch::PtrToAddr(target_bb->GetStartPtr());
    auto align_size = kAlign - (start_addr % kAlign);

    std::printf("Aligning loop at 0x%lx with %ld bytes\n", start_addr, align_size);

    if (align_size == kAlign) {
      return;
    }

    if (align_size % 2 != 0) {
      std::printf("WARN: align_size %% 2 != 0\n");
      return;
    }

    auto& code = bb_col_->GetCodeStore();
    auto range = code.AllocRange(align_size);
    arch::InsertNops(range.GetStartPtr(), align_size);

    auto* new_bb = &bb_col_->Insert(range.GetStartPtr(), range.GetEndPtr());
    new_bb->SetNext(target_bb->GetNext());
    target_bb->SetNext(new_bb);
  }

 private:
  static constexpr auto kAlign = 32;

  BasicBlockCollection* bb_col_;
};

}  // namespace bb
