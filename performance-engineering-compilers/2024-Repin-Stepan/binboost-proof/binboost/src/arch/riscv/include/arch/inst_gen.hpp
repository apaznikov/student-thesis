#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace bb::arch {

namespace {

// @todo refactor decoding
// https://gist.github.com/michaeljclark/5b4d2c40d14c23c5d77b

static constexpr std::uint32_t BuildJ(std::uint8_t opcode, std::uint8_t rd,
                                      std::uint32_t imm) {
  // 31      30        20      19         11 6
  // imm[20] imm[10:1] imm[11] imm[19:12] rd opcode

  std::uint32_t instr = 0;
  instr |= opcode << 0;
  instr |= rd << 7;

  // Всего imm 21 бит; 0 бит пропускается.
  std::uint32_t imm_19_12 = imm & 0x00ff000;  // 8 бит
  std::uint32_t imm_11 = imm & 0x0000800;     // 1 бит
  std::uint32_t imm_10_1 = imm & 0x00007fe;   // 10 бит
  std::uint32_t imm_20 = imm & 0x0100000;     // 1 бит

  instr |= imm_19_12 << 0;  // с 12 позиции
  instr |= imm_11 << 9;     // с 20 позиции
  instr |= imm_10_1 << 20;  // с 21 позиции
  instr |= imm_20 << 11;    // с 31 позиции

  return instr;
}

static constexpr std::uint32_t BuildU(std::uint8_t opcode, std::uint8_t rd,
                                      std::uint32_t imm) {
  std::uint32_t instr = 0;
  instr |= opcode << 0;
  instr |= rd << 7;
  instr |= (imm & 0xfffff000) << 12;
  return instr;
}

static constexpr std::uint32_t BuildI(std::uint8_t opcode, std::uint8_t rd,
                                      std::uint8_t funct3, std::uint8_t rs1,
                                      std::uint16_t imm) {
  // 31        19  14     11 6
  // imm[11:0] rs1 funct3 rd opcode

  std::uint32_t instr = 0;
  instr |= opcode << 0;
  instr |= rd << 7;
  instr |= funct3 << 12;
  instr |= rs1 << 15;
  instr |= (imm & 0xfff) << 20;
  return instr;
}

static constexpr std::uint16_t BuildCjType(std::uint8_t opcode,
                                           std::uint16_t offset,
                                           std::uint8_t funct3) {
  std::uint16_t instr = 0;
  instr |= opcode << 0;

  std::uint16_t imm_5 = offset & (1 << 5);
  std::uint16_t imm_3_1 = offset & 0xe;
  std::uint16_t imm_7 = offset & (1 << 7);
  std::uint16_t imm_6 = offset & (1 << 6);
  std::uint16_t imm_10 = offset & (1 << 10);
  std::uint16_t imm_9_8 = offset & 0x300;
  std::uint16_t imm_4 = offset & (1 << 4);
  std::uint16_t imm_11 = offset & (1 << 11);

  instr |= imm_5 >> 3; // Позиция 2
  instr |= imm_3_1 << 2; // Позиция 3-5
  instr |= imm_7 >> 1; // Позиция 6
  instr |= imm_6 << 1; // Позиция 7
  instr |= imm_10 >> 2; // Позиция 8
  instr |= imm_9_8 << 1; // Позиция 9-10
  instr |= imm_4 << 7; // Позиция 11
  instr |= imm_11 << 1; // Позиция 12

  instr |= funct3 << 13;
  return instr;
}
}  // namespace

static constexpr std::uint32_t BuildJal(std::uint8_t rd, std::uint32_t imm) {
  return BuildJ(0x6f, rd, imm);
}

static constexpr std::uint16_t BuildCj(std::uint16_t offset) {
  return BuildCjType(0x1, offset, 0x5);
}

static constexpr std::uint32_t BuildLui(std::uint8_t rd, std::uint16_t imm) {
  return BuildU(0x37, rd, imm);
}

static constexpr std::uint32_t BuildAddi(std::uint8_t rd, std::uint8_t rs,
                                         std::uint16_t imm) {
  return BuildI(0x13, rd, 0x0, rs, imm);
}

static constexpr std::uint32_t BuildOri(std::uint8_t rd, std::uint8_t rs,
                                        std::uint16_t imm) {
  return BuildI(0x13, rd, 0x6, rs, imm);
}

static constexpr std::uint32_t BuildSlli(std::uint8_t rd, std::uint8_t rs,
                                         std::uint16_t imm) {
  return BuildI(0x13, rd, 0x1, rs, imm);
}

static constexpr std::uint32_t BuildJalr(std::uint8_t rd, std::uint8_t base,
                                         std::uint16_t offset) {
  return BuildI(0x67, rd, 0x0, base, offset);
}

static inline void gen_indirect_jmp_to(std::uint8_t* memory,
                                       std::uintptr_t addr) {
  auto copy_instr = [&memory](std::uint32_t instr) {
    std::memcpy(memory, &instr, 4);
    memory += 4 * sizeof(std::uint8_t);
  };

  constexpr std::uint8_t kRegZero = 0x0;
  constexpr std::uint8_t kRegT0 = 0x5;

  constexpr int kHigh20BitsShift = 32 + 12;

  // lui t0, <addr-high-20>
  auto lui = BuildLui(kRegT0, (addr >> kHigh20BitsShift) & 0xfffff);
  copy_instr(lui);

  // Оставшиеся 44 бита загружаем 4-мя операциями OR со сдвигом (каждая -- по 11
  // битов).
  //
  constexpr int kBitsPerOp = 11;  // Одним OR загружаем 11 бит.
  constexpr int kOpsNeeded = ((64 - 20) / 11);
  static_assert(kOpsNeeded == 4);

  for (int i = 0; i < kOpsNeeded; ++i) {
    std::uint16_t high11 = (addr >> (kOpsNeeded - i - 1) * kBitsPerOp) & 0x7ff;

    // ori t0, t0, <high11>
    auto ori = BuildOri(kRegT0, kRegT0, high11);
    copy_instr(ori);

    // Последний раз сдвигать не надо.
    if (i != kOpsNeeded - 1) {
      // slli t0, t0, <kBitsPerOp>
      auto slli = BuildSlli(kRegT0, kRegT0, kBitsPerOp);
      copy_instr(slli);
    }
  }

  // jalr zero, 0(t0)
  auto jalr = BuildJalr(kRegZero, kRegT0, 0);
  copy_instr(jalr);
}

}  // namespace bb::arch
