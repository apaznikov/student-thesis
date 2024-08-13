#include <cstdio>

#include "binboost/bb.h"

class BubbleSort2 {
 public:
  static constexpr int kBound = 5000 * 15;

  __attribute__((noinline)) static void Inner(int* x) {
    auto limit1 = kBound - 1;
    BB_JIT_BEGIN();
    for (int i = 1; i <= limit1; ++i) {
      for (int j = i; j <= kBound; ++j) {
        if (x[i] > x[j]) {
          int temp = x[j];
          x[j] = x[i];
          x[i] = temp;
        }
      }
    }
    BB_JIT_END();
  }

  static bool Test() {
    int* x = new int[kBound + 1];
    int j = 99999;
    int limit = kBound - 2;
    int i = 1;

    do {
      x[i] = j & 32767;
      x[i + 1] = (j + 11111) & 32767;
      x[i + 2] = (j + 22222) & 32767;
      j = j + 33333;
      i = i + 3;
    } while (i <= limit);

    x[kBound - 1] = j;
    x[kBound] = j;

    Inner(x);

    for (i = 0; i < kBound - 1; ++i) {
      if (x[i] > x[i + 1]) {
        return false;
      }
    }

    return true;
  }
};

int main() {
  auto res = BubbleSort2::Test();
  printf("Result %d\n", static_cast<int>(res));
  return 0;
}
