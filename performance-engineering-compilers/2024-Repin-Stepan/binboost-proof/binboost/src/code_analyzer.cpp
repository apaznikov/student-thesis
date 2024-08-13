#include "code_analyzer.hpp"

#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/mman.h>
#include <unistd.h>

#include "arch/inst_gen.hpp"
#include "basic_block_parser.hpp"

namespace bb {

static constexpr int kPageSize = 4096;

std::uintptr_t CodeAnalyzer::CopyCode() {
  auto* mapped_ptr = mmap(nullptr, kPageSize, PROT_READ | PROT_WRITE,
                          (MAP_ANONYMOUS | MAP_PRIVATE), -1, 0);

  auto* memory = reinterpret_cast<std::uint8_t*>(mapped_ptr);

  CopyCode(memory);

  int rc = mprotect(mapped_ptr, 4096, PROT_EXEC);
  if (rc != 0) {
    std::fprintf(stderr, "Panic: call to mprotect() failed: %d\n", rc);
    std::exit(1);
  }

  return std::bit_cast<std::uintptr_t>(memory);
}

void CodeAnalyzer::CopyCode(std::uint8_t* memory) {
  const BasicBlock* bb = &*bbs_->begin();
  while (bb != nullptr) {
    std::memcpy(memory, bb->GetStartPtr(), bb->GetSize());
    memory += bb->GetSize();
    bb = bb->GetNext();
  }
  auto to_addr = bbs_->GetOriginalEnd();
  arch::gen_indirect_jmp_to(memory, to_addr);
}

}  // namespace bb
