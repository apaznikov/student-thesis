#pragma once

#include "instr.hpp"

namespace bb::arch {

class Decoder {
 public:
  explicit Decoder(InstrData* instr) : instr_(instr) {}

  bool IsCompressed() const;
  int GetInstrSizeMultiplier() const;

  template <InstrCategory TCat>
  bool TryDecode(Instr<TCat>* instr) const;

 private:
  InstrData* instr_;
};

}  // namespace bb::arch
