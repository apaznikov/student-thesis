#pragma once

#include "basic_block_parser.hpp"

namespace bb {

class CodeAnalyzer {
 public:
  explicit CodeAnalyzer(const BasicBlockCollection* bbs) : bbs_(bbs) {}

  std::uintptr_t CopyCode();

 private:
  void CopyCode(std::uint8_t* memory);

  const BasicBlockCollection* bbs_;
};

}  // namespace bb
