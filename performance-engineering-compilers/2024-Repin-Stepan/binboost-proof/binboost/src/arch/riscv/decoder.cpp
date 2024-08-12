#include "arch/decoder.hpp"

#include <cstdio>

#include "arch/instr.hpp"

namespace bb::arch {

namespace {

template <typename T>
struct SignExtend {
  static constexpr auto kWidth = T::kMaxMsb - 1;
  static constexpr std::int64_t Decode(std::uint32_t instr) {
    struct {
      std::int64_t x : kWidth;
    } s;
    return s.x = T::Decode(instr);
  }
};

template <int MsbOff, int LsbOff>
struct Bit {
  static_assert(MsbOff >= LsbOff);

  static constexpr int kMsbOffset = MsbOff;
  static constexpr int kLsbOffset = LsbOff;
  static constexpr int kWidth = MsbOff - LsbOff + 1;

  static constexpr std::uint32_t Decode(std::uint32_t val) {
    return val & ((1 << kWidth) - 1);
  }
};

template <int Count>
struct Skip {
  static constexpr int kWidth = Count;
  static constexpr int kMsbOffset = 0;
  static constexpr int kLsbOffset = 0;
  static constexpr std::uint32_t Decode(std::uint32_t) { return 0; }
};

template <int A, int B>
struct Max {
  static constexpr auto kValue = (A > B) ? A : B;
};

template <typename... Bits>
struct Format {
  static constexpr int kWidth = 0;
  static constexpr int kMaxMsb = 0;

  static constexpr std::uint32_t Decode(std::uint32_t) { return 0; }
};

template <typename BitHead, typename... Bits>
struct Format<BitHead, Bits...> {
  using BitTail = Format<Bits...>;

  static constexpr int kWidth = BitHead::kWidth + BitTail::kWidth;
  static constexpr int kMaxMsb =
      Max<BitHead::kMsbOffset, BitTail::kMaxMsb>::kValue;

  static constexpr std::uint32_t Decode(std::uint32_t val) {
    // Вытаскиваем из val бит, соответствующий порядковому BitHead.
    auto bits = BitHead::Decode(val >> BitTail::kWidth);

    // Передвигаем бит на нужную позицию и добавляем значения
    // остальных битов.
    return (bits << BitHead::kLsbOffset) | BitTail::Decode(val);
  }
};

template <int MsbOff, int LsbOff = MsbOff>
using B = Bit<MsbOff, LsbOff>;

template <int Count>
using S = Skip<Count>;

//
//     15  13 12                          2 0    1
// CJ  funct3 offset[11|4|9:8|10|6|7|3:1|5] opcode
//
struct CjFormat {
  static constexpr auto ReadOp(std::uint32_t instr) {
    return Format<B<1, 0>>::Decode(instr);
  }

  static constexpr auto ReadFunct3(std::uint32_t instr) {
    return Format<B<2, 0>, S<13>>::Decode(instr);
  }

  static constexpr auto ReadOffset(std::uint32_t instr) {
    return SignExtend<  //
        Format<B<11>, B<4>, B<9, 8>, B<10>, B<6>, B<7>, B<3, 1>, B<5>,
               S<2>>>  //
        ::Decode(instr);
  }

  static constexpr bool IsValid(std::uint32_t instr) {
    return ReadOp(instr) == 0x1 && ReadFunct3(instr) == 0b101;
  }
};

//    31      30        20      19         11 6
// J  imm[20] imm[10:1] imm[11] imm[19:12] rd opcode
struct JFormat {
  static constexpr auto ReadOp(std::uint32_t instr) {
    return Format<B<6, 0>>::Decode(instr);
  }

  static constexpr auto ReadRd(std::uint32_t instr) {
    return Format<B<4, 0>, S<7>>::Decode(instr);
  }

  static constexpr auto ReadImm(std::uint32_t instr) {
    return SignExtend<Format<B<20>, B<10, 1>, B<11>, B<19, 12>, S<12>>>  //
        ::Decode(instr);
  }

  static constexpr auto ReadOffset(std::uint32_t instr) {
    return ReadImm(instr);
  }

  static constexpr bool IsValid(std::uint32_t instr) {
    auto op = ReadOp(instr);
    return op == 0x6f || op == 0x67;
  }
};

//    31      30        24  19  14     11       7       6
// B  imm[12] imm[10:5] rs2 rs1 funct3 imm[4:1] imm[11] opcode
//
// offset = imm[12,10:5,11,4:1]
// src1 = rs1
// src2 = rs2
struct BFormat {
  static constexpr auto ReadOp(std::uint32_t instr) {
    return Format<B<6, 0>>::Decode(instr);
  }

  static constexpr auto ReadSrc1(std::uint32_t instr) {
    return Format<B<4, 0>, S<15>>::Decode(instr);
  }

  static constexpr auto ReadSrc2(std::uint32_t instr) {
    return Format<B<4, 0>, S<20>>::Decode(instr);
  }

  static constexpr auto ReadFunct3(std::uint32_t instr) {
    return Format<B<2, 0>, S<12>>::Decode(instr);
  }

  static constexpr auto ReadOffset(std::uint32_t instr) {
    return SignExtend<Format<B<12>, B<10, 5>, S<13>, B<4, 1>, B<11>, S<7>>>  //
        ::Decode(instr);
  }

  static constexpr auto IsValid(std::uint32_t instr) {
    return ReadOp(instr) == 0b1100011;
  }
};

template <int Funct3>
struct ConcreteBFormat : BFormat {
  static constexpr auto IsValid(std::uint32_t instr) {
    auto funct3 = ReadFunct3(instr);
    return BFormat::IsValid(instr) && funct3 == Funct3;
  }
};

using BeqFormat = ConcreteBFormat<0b000>;
using BneFormat = ConcreteBFormat<0b001>;
using BltFormat = ConcreteBFormat<0b100>;
using BgeFormat = ConcreteBFormat<0b101>;
using BltuFormat = ConcreteBFormat<0b110>;
using BgeuFormat = ConcreteBFormat<0b111>;

}  // namespace

template <>
bool Decoder::TryDecode<InstrCategory::Jump>(JumpInstr* instr) const {
  std::uint16_t instr16 = *instr_;
  std::uint32_t instr32 = (*(instr_ + 1) << 16) | (instr16);

  auto try_decode = [this, instr]<typename Format>(auto instr_data) {
    if (!Format::IsValid(instr_data)) {
      return false;
    }
    instr->data = instr_;
    instr->target = instr_ + (Format::ReadOffset(instr_data) / 2);
    return true;
  };

  return                                           //
      try_decode.operator()<CjFormat>(instr16) ||  //
      try_decode.operator()<JFormat>(instr32) ||   //
      try_decode.operator()<BFormat>(instr32);
}

bool Decoder::IsCompressed() const {
  // 32-битные инструкции начинаются с 0b11 в младших битах
  // 16-битные -- с 0b00, 0b01, 0b10
  return (*instr_ & 0x3) != 0b11;
}

int Decoder::GetInstrSizeMultiplier() const { return IsCompressed() ? 1 : 2; }

namespace test {

struct FormatTest {
  static_assert(Format<B<3>>::Decode(0b1) == 0b1000);
  static_assert(Format<B<3>, B<1>>::Decode(0b11) == 0b1010);
  static_assert(Format<B<3>, B<1, 0>>::Decode(0b111) == 0b1011);

  //
  //      12         10  6               2
  // CB   offset[8|4:3]  offset[7:6|2:1|5]
  //
  using Test1Format = Format<B<8>, B<4, 3>, S<3>, B<7, 6>, B<2, 1>, B<5>, S<2>>;

  // c.bnez x10, 52
  // 0xe915 = 0b1110'1001'0001'0101
  static_assert(Test1Format::Decode(0xe915) == 52);
};

struct CFormatTest {
  // c.j -4
  // 0xbff5 = 0b1011'1111'1111'0101
  static constexpr auto kI1 = 0xbff5;
  static_assert(CjFormat::ReadOp(kI1) == 1);
  static_assert(CjFormat::ReadFunct3(kI1) == 5);
  static_assert(CjFormat::ReadOffset(kI1) == -4);
  static_assert(CjFormat::IsValid(kI1));
};

struct JFormatTest {
  // jal x1, 23866
  // 0x53b050ef = 0b0101'0011'1011'0000'0101'0000'1110'1111
  static constexpr auto kI1 = 0x53b050ef;
  static_assert(JFormat::ReadOp(kI1) == 0b1101111);
  static_assert(JFormat::ReadRd(kI1) == 1);
  static_assert(JFormat::ReadOffset(kI1) == 23866);
  static_assert(JFormat::IsValid(kI1));
};

struct BFormatTest {
  // bge x15, x14, 68
  // 0x04e7d263 = 0b0000'0100'1110'0111'1101'0010'0110'0011
  static constexpr auto kI1 = 0x04e7d263;
  static_assert(BFormat::ReadOp(kI1) == 0b1100011);
  static_assert(BFormat::ReadSrc1(kI1) == 0b01111);
  static_assert(BFormat::ReadSrc2(kI1) == 0b01110);
  static_assert(BFormat::ReadFunct3(kI1) == 0b101);
  static_assert(BFormat::ReadOffset(kI1) == 68);
  static_assert(BFormat::IsValid(kI1));

  // bge x15, x12, -14
  // 0xfec799e3 = 0b1111'1110'1100'0111'10011001'11100011
  static constexpr auto kI2 = 0xfec799e3;
  static_assert(BFormat::ReadOp(kI2) == 0b1100011);
  static_assert(BFormat::ReadSrc1(kI2) == 0b01111);
  static_assert(BFormat::ReadSrc2(kI2) == 0b01100);
  static_assert(BFormat::ReadFunct3(kI2) == 0b001);
  static_assert(BFormat::ReadOffset(kI2) == -14);
  static_assert(BFormat::IsValid(kI2));
};

}  // namespace test

}  // namespace bb::arch
