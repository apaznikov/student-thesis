#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <list>

#include "arch/instr.hpp"

namespace bb {

// Диапазон кода.
struct CodeRange {
  /// Виртуальный адрес начала диапазона (включительно).
  std::uintptr_t start;
  /// Виртуальный адрес конца диапазона (включительно).
  std::uintptr_t end;

  /// Размер диапазона.
  std::size_t GetSize() const { return end - start; }

  // Представляет ли структура валидный диапазон.
  bool IsValid() const { return end > 0 && start <= end; }

  // Возвращает указатель на начало диапазона.
  arch::InstrData* GetStartPtr() const {
    return arch::AddrToPtr<arch::InstrData*>(start);
  }

  // Возвращает указатель на конец диапазона.
  arch::InstrData* GetEndPtr() const {
    return arch::AddrToPtr<arch::InstrData*>(end);
  }
};

// Динамическое хранилище памяти для инструкций, которые создаются в процессе
// оптимизации кода.
class CodeStore {
public:
  CodeStore();

  // Выделяет диапазон инструкций размера size.
  CodeRange AllocRange(std::size_t size);

private:
  static constexpr auto kMaxSize = 512;

  std::size_t end_ = 0;
  std::vector<std::byte> code_;
};

// Базовый блок.
//
// Последовательность инструкций, непрерывающихся инструкциями перехода или
// метками.
//
// Может иметь 0 или больше родителей и 1 или 2 дочерних узла.
//
// Может ссылать на инструкции в оптимизируемой программы или на инструкции из
// CodeStore. В любом случае, всегда иммутабелен.
class BasicBlock {
 public:
  BasicBlock() = default;

  // Создает новый ББ, представляющий инструкции с адреса s до адреса e.
  BasicBlock(const arch::InstrData* s, const arch::InstrData* e)
      : start_(s), size_(CalculateSizeFromStartAndEnd(s, e)) {}

  // Возвращает адрес на первую инструкцию.
  const arch::InstrData* GetStartPtr() const { return start_; }
  // Возвращает адрес на инструкцию, идущую за последней.
  const arch::InstrData* GetEndPtr() const { return start_ + size_; }
  // Возвращает размер блока.
  std::size_t GetSize() const { return size_ * sizeof(arch::InstrData); }
  // Возвращает указатель на следующий ББ.
  BasicBlock* GetNext() { return next_; }
  // Возвращает константный указатель на следующий ББ.
  const BasicBlock* GetNext() const { return next_; }
  // Возвращает указатель на ББ, в который совершается переход из данного.
  BasicBlock* GetJumpTarget() { return jmp_target_; }

  // Устанавливает следующий ББ.
  void SetNext(BasicBlock* bb) { next_ = bb; }
  // Устанавливает ББ, в который совершается переход из данного.
  void SetJumpTarget(BasicBlock* bb) { jmp_target_ = bb; }

 private:
  std::size_t CalculateSizeFromStartAndEnd(const arch::InstrData* s,
                                           const arch::InstrData* e) {
    return e - s;
  }

  const arch::InstrData* start_;
  std::size_t size_;

  BasicBlock* jmp_target_ = nullptr;
  BasicBlock* next_ = nullptr;
};

// Представляет CFG -- граф BasicBlock
class BasicBlockCollection {
 public:
  // Контейнер, в котором хранятся BasicBlock.
  using Container =
      std::list<BasicBlock>;

  // Итераторы CFG.
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;

  BasicBlockCollection() = default;

  // Создает CFG, представляющий код по указанному диапазону.
  explicit BasicBlockCollection(std::uintptr_t original_start,
                                std::uintptr_t original_end)
      : original_start_(original_start), original_end_(original_end) {}

  // Вставляет в CFG новый ББ.
  BasicBlock& Insert(arch::InstrData* start, arch::InstrData* end);

  // Возвращает корень CFG.
  BasicBlock& GetFirst() { return *blocks_.begin(); }
  // Выводит отладочную информацию о CFG.
  void Dump(FILE* f);

  // Возвращает число ББ в CFG.
  std::size_t Size() const { return blocks_.size(); }

  // Проверяет корректен ли CFG.
  bool IsValid() const { return blocks_.size() > 0; }
  // Возвращает конец исходного кода, для которого построен CFG.
  std::uintptr_t GetOriginalEnd() const { return original_end_; }

  // Возвращает итераторы на начало и конец CFG.
  auto begin() const { return blocks_.begin(); }
  auto end() const { return blocks_.end(); }

  // Возвращает хранилище инструкций.
  CodeStore& GetCodeStore() { return code_store_; }

 private:
  std::uintptr_t original_start_;
  std::uintptr_t original_end_;
  Container blocks_;
  CodeStore code_store_;
};

// Выполняет построение CFG.
class BasicBlockParser {
 public:
  explicit BasicBlockParser(std::uintptr_t start) : start_(start) {}

  // Строит CFG.
  BasicBlockCollection Parse();

 private:
  CodeRange ParseRange();
  std::uintptr_t start_;
};

}  // namespace bb
