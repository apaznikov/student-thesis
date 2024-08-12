#pragma once

#include <bit>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace bb::arch {

namespace concepts {

template <class T>
concept Pointer = std::is_pointer_v<T>;

}  // namespace concepts

using InstrData = std::uint16_t;

template <concepts::Pointer T>
static inline std::uintptr_t PtrToAddr(const T ptr) {
  return std::bit_cast<std::uintptr_t>(ptr);
}

// @todo move to utils
template <concepts::Pointer T>
static inline T AddrToPtr(std::uintptr_t addr) {
  return reinterpret_cast<T>(addr);
}

enum class InstrCategory {
  Base,
  Jump,
};

enum class InstrType {
  Unknown,
  C,
  J,
};

enum class InstrName {
  Unknown,
  Addi,
  Jal,
  Jalr,
};

struct InstrInfo {
  InstrName name;
  InstrType type;

  bool Is4Byte() { return type != InstrType::C; }
  bool IsJump() const {
    //
    // @todo C-type
    return type == InstrType::J;
  }
};

class InstrBuffer {
 public:
  using Container = std::vector<InstrInfo>;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;

  void Add(InstrInfo instr_info) { instrs_.push_back(instr_info); }

  auto begin() const { return instrs_.begin(); }
  auto end() const { return instrs_.end(); }

 private:
  Container instrs_;
};

template <InstrCategory TCat>
struct Instr {
  static constexpr auto kCategory = TCat;

  InstrData* data;
};

template <>
struct Instr<InstrCategory::Base> {
  static constexpr auto kCategory = InstrCategory::Base;

  InstrName GetName();

  InstrData* data;
};

template <>
struct Instr<InstrCategory::Jump> {
  static constexpr auto kCategory = InstrCategory::Jump;

  InstrName GetName();
  std::uintptr_t GetTarget() { return PtrToAddr(target); }
  std::uintptr_t GetAddr() { return PtrToAddr(data); }

  InstrData* data;
  InstrData* target = 0;
};

using BaseInstr = Instr<InstrCategory::Base>;
using JumpInstr = Instr<InstrCategory::Jump>;

}  // namespace bb::arch
