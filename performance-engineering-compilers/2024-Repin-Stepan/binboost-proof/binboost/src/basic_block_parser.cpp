#include "basic_block_parser.hpp"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <map>
#include <optional>
#include <utility>

#include "binboost/bb.h"

#include "arch/decoder.hpp"
#include "arch/instr.hpp"
#include "arch/intrinsics.hpp"

namespace bb {

namespace {

class JumpTable {
 public:
  enum class PointType {
    Start,
    End,
    Label,
    Jump,
  };

  struct JumpInfo {
    PointType type;
    arch::InstrData* place;
    arch::InstrData* bind;
    BasicBlock* bb;
  };

  using PointContainer =
      std::map<arch::InstrData*, JumpInfo, std::less<arch::InstrData*>>;

  using iterator = PointContainer::const_iterator;

  static JumpTable ParseRange(const CodeRange& code);

  std::optional<JumpInfo> FindByAddr(arch::InstrData* instr, PointType type);

  void Dump(std::FILE* f) const;

  auto begin() { return table_.begin(); }
  auto end() { return table_.end(); }

 private:
  void Add(arch::InstrData* instr_ptr, PointType type,
           arch::InstrData* sec_addr);

  PointContainer table_;
};

JumpTable JumpTable::ParseRange(const CodeRange& code) {
  JumpTable tbl;

  tbl.Add(code.GetStartPtr(), PointType::Start, nullptr);

  auto* cur_instr = code.GetStartPtr();
  auto* end_instr = code.GetEndPtr();
  arch::JumpInstr instr;
  while (true) {
    auto decoder = arch::Decoder(cur_instr);
    auto is_jmp = decoder.TryDecode<arch::InstrCategory::Jump>(&instr);

    if (is_jmp) {
      tbl.Add(cur_instr, PointType::Jump, instr.target);
      tbl.Add(instr.target, PointType::Label, cur_instr);
    }

    auto instr_size = decoder.GetInstrSizeMultiplier();
    cur_instr += instr_size;
    if (cur_instr == end_instr) {
      break;
    }
  }

  tbl.Add(code.GetEndPtr(), PointType::End, nullptr);

  return tbl;
}

void JumpTable::Add(arch::InstrData* instr_ptr, PointType type,
                    arch::InstrData* sec_addr) {
  table_.emplace(instr_ptr, JumpInfo{type, instr_ptr, sec_addr, nullptr});
}

std::optional<JumpTable::JumpInfo> JumpTable::FindByAddr(arch::InstrData* instr,
                                                         PointType type) {
  if (table_.contains(instr)) {
    auto info = table_.find(instr)->second;
    return (info.type == type) ? std::optional{info} : std::nullopt;
  }
  return std::nullopt;
}

void JumpTable::Dump(std::FILE* f) const {
  auto get_type_str = [](PointType type) {
    switch (type) {
      case PointType::Start:
        return "Start";
      case PointType::End:
        return "End";
      case PointType::Jump:
        return "Jump";
      case PointType::Label:
        return "Label";
    }
    return "<unknown>";
  };
  std::fprintf(f, "JumpTable dump:\n");
  for (auto& [instr, info] : table_) {
    std::fprintf(f, "  %s\t<0x%lx> -> <0x%lx>\n", get_type_str(info.type),
                 arch::PtrToAddr(instr), arch::PtrToAddr(info.bind));
  }
}

}  // namespace

CodeStore::CodeStore() { code_.resize(kMaxSize); }

CodeRange CodeStore::AllocRange(std::size_t size) {
  assert((code_.size() >= end_ + size) &&
         "Not enought memory to allocate code range");
  auto start = code_.data() + end_;
  end_ += size;
  return {std::bit_cast<std::uintptr_t>(start),
          std::bit_cast<std::uintptr_t>(start + size)};
}

BasicBlock& BasicBlockCollection::Insert(arch::InstrData* start,
                                         arch::InstrData* end) {
  return *blocks_.emplace(blocks_.end(), BasicBlock{start, end});
}

void BasicBlockCollection::Dump(FILE* f) {
  // Осторожно с порядком байтов. Dump() выводит все одиночными байтами, поэтому
  // не учитывает порядок байтов внутри инструкции.
  auto dump_as_hex = [](const arch::InstrData* begin,
                        const arch::InstrData* end) {
    auto begin_b = reinterpret_cast<const std::byte*>(begin);
    auto end_b = reinterpret_cast<const std::byte*>(end);
    while (begin_b != end_b) {
      std::printf("  ");
      for (auto i = 0; i < 8; ++i) {
        std::printf("%02x ", static_cast<int>(*begin_b));
        ++begin_b;
        if (begin_b == end_b) {
          break;
        }
      }
      std::puts("");
    }
  };

  int i = 1;
  BasicBlock* bb = &GetFirst();
  while (bb != nullptr) {
    std::fprintf(f, "BB%d:\n", i++);

    auto start = arch::PtrToAddr(bb->GetStartPtr());
    auto end = arch::PtrToAddr(bb->GetEndPtr());

    auto jmp_target = bb->GetJumpTarget();
    auto jmp_target_addr =
        jmp_target ? arch::PtrToAddr(jmp_target->GetStartPtr()) : 0;

    std::fprintf(f, "  start=0x%lx, end=0x%lx, target=0x%lx, size=%ldB\n",
                 start, end, jmp_target_addr, bb->GetSize());
    dump_as_hex(bb->GetStartPtr(), bb->GetEndPtr());

    bb = bb->GetNext();
  }
}

BasicBlockCollection BasicBlockParser::Parse() {
  auto range = ParseRange();
  std::printf("Found code range from=0x%lx to=0x%lx size=%ldB\n", range.start,
              range.end, range.GetSize());
  if (!range.IsValid()) {
    return {};
  }
  std::printf("Building JumpTable...\n");
  auto jtbl = JumpTable::ParseRange(range);
  std::printf("JumpTable is built\n");
  jtbl.Dump(stdout);

  BasicBlockCollection bb_col(range.start, range.end);

  std::printf("Building CFG...\n");
  BasicBlock* prev_bb = nullptr;
  for (auto it = jtbl.begin(); it != jtbl.end(); ++it) {
    auto type = it->second.type;
    if (type == JumpTable::PointType::End) {
      break;
    }

    auto* start_ptr = it->first;
    auto* end_ptr = std::next(it)->first;

    auto* bb = &bb_col.Insert(start_ptr, end_ptr);

    if (type == JumpTable::PointType::Jump) {
      it->second.bb = prev_bb;
    } else if (type == JumpTable::PointType::Label) {
      it->second.bb = bb;
    }

    if (prev_bb != nullptr) {
      prev_bb->SetNext(bb);
    }
    prev_bb = bb;
  }

  for (auto it = jtbl.begin(); it != jtbl.end(); ++it) {
    auto type = it->second.type;
    if (type == JumpTable::PointType::End) {
      break;
    }

    if (type == JumpTable::PointType::Jump) {
      auto cur_bb = it->second.bb;
      auto jmp_target_info =
          jtbl.FindByAddr(it->second.bind, JumpTable::PointType::Label);
      assert(jmp_target_info && "Failed to find jump target BB");
      cur_bb->SetJumpTarget(jmp_target_info->bb);
    }
  }

  return bb_col;
}

CodeRange BasicBlockParser::ParseRange() {
  auto jit_end_addr = arch::PtrToAddr(&bb_jit_end);
  auto* jit_end_ptr = arch::AddrToPtr<arch::InstrData*>(jit_end_addr);

  static constexpr int kMaxCodeSizeIn16Bit = 512;

  CodeRange range;
  range.start = start_;

  auto* code_ptr = arch::AddrToPtr<arch::InstrData*>(start_);
  int iteration = 0;
  while (true) {
    auto decoder = arch::Decoder(code_ptr);
    arch::Instr<arch::InstrCategory::Jump> jmp_instr;

    auto is_jmp = decoder.TryDecode<arch::InstrCategory::Jump>(&jmp_instr);
    auto is_call_to_end = is_jmp && jmp_instr.target == jit_end_ptr;

    if (is_call_to_end) {
      break;
    }

    code_ptr += arch::Decoder(code_ptr).GetInstrSizeMultiplier();
    ++iteration;
    if (iteration >= kMaxCodeSizeIn16Bit) {
      std::printf("Warning: code range too big (>= %d 16 bit words)\n",
                  kMaxCodeSizeIn16Bit);
      return {};
    }
  }

  auto end = std::bit_cast<std::uintptr_t>(code_ptr);
  range.end = end;
  return range;
}

}  // namespace bb
