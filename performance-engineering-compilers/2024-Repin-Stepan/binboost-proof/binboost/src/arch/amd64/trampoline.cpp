// @todo Переделать в файл trampoline.S

#ifndef BB_EXCLUDE_TRAMPOLINE

// clang-format off

/// Обычная функция, но сразу после эпилога вызывает не `ret`, а `jmp`.
__asm__(
    ".align 64\n"
    ".global bb_jit_begin\n"
    "bb_jit_begin:\n"
        "pushq %rbp\n"
        "movq  %rsp, %rbp\n"
        "subq $8, %rsp\n"
        "pushq %rbx\n"
        "xorq %rdi, %rdi\n"
        "callq bb_jit_begin_impl\n"
        "popq %rbx\n"
        "popq  %rbp\n"
        "popq  %rbp\n"
        "addq $8, %rsp\n"
        "jmpq  *%rax\n"
);
// clang-format on

#endif // !BB_EXCLUDE_TRAMPOLINE
