#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/mman.h>

#define POLYGON_SIZE 4096

#define push(bin, opcode) memcpy(bin, opcode, sizeof(opcode) - 1) + sizeof(opcode) - 1
#define push_arr(bin, opcode) memcpy(bin, opcode, sizeof(opcode)) + sizeof(opcode)


union int64
{
    int64_t value;
    char bytes[sizeof(int64_t)];
};


static inline uint64_t xorshift64()
{
    static uint64_t x = 1;

    x ^= x << 13;
    x ^= x >>  7;
    x ^= x << 17;

    return x;
}


double get_timestamp()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (double) now.tv_sec + (double) now.tv_nsec / 1e9;
}


double measure_opcodes(const uint8_t *bin, size_t len, uint64_t argument)
{
    // Allocating virtual memory for binary
    void *polygon = mmap(NULL, POLYGON_SIZE, PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (polygon == MAP_FAILED)
    {
        puts("Error: failed to allocate memory with mmap()");
        exit(1);
    }

    // Copying binary to polygon
    memcpy(polygon, bin, len);

    double measure_start = get_timestamp();

    // Running binary
    ((void (*) (uint64_t)) polygon)(argument);

    double measure_duration = get_timestamp() - measure_start;

    // Deallocating binary space
    munmap(polygon, POLYGON_SIZE);

    return measure_duration;
}


void test_loop_alignment(bool loop)
{
    if (loop)
        puts("Loop alignment (LOOP)\n");
    else
        puts("Loop alignment (DEC + JNZ)\n");

    printf("    ");

    for (uint8_t nop_count = 0; nop_count <= 5; ++nop_count)
        printf("%5" PRIu8 " ", nop_count);

    putchar('\n');

    for (size_t alignment = 59; alignment <= 64; ++alignment)
    {
        printf("%zu:  ", alignment);

        for (uint8_t nop_count = 0; nop_count <= 5; ++nop_count)
        {
            uint8_t bin_start[128];
            uint8_t *bin = bin_start;

            bin = push(bin, "\x48\x89\xF9");        // mov rcx, rdi

            // Align
            for (size_t i = 3; i < alignment; ++i)
                bin = push(bin, "\x90");            // nop

            // NOPs in loop
            for (size_t i = 0; i < nop_count; ++i)
                bin = push(bin, "\x90");            // nop

            if (loop)
            {
                bin = push(bin, "\xE2");                // loop loop_begin
                *bin++ = 0 - nop_count - 2;
            }
            else
            {
                bin = push(bin, "\x48\xFF\xC9");        // dec rcx
                bin = push(bin, "\x75");                // jnz loop_begin
                *bin++ = 0 - nop_count - 3 - 2;
            }

            bin = push(bin, "\xC3");                // ret

            printf("%.2lf  ", measure_opcodes(bin_start, bin - bin_start, 1000000000));
            fflush(stdout);
        }

        putchar('\n');
    }

    putchar('\n');
}


void test_unaligned_read_workload(const char *test_description, int64_t area_size, int64_t passes)
{
    union int64 iterations;

    // Allocate area to operate in
    uint64_t *area = mmap(NULL, area_size * sizeof(uint64_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (area == MAP_FAILED)
    {
        puts("Error: failed to allocate memory with mmap()");
        exit(1);
    }

    // Fill area
    for (size_t i = 0; i < area_size; ++i)
        area[i] = xorshift64();

    uint8_t bin_start[128];
    uint8_t *bin = bin_start;

    bin = push(bin, "\x49\xBA");            // mov r10, iterations  ; Outer loop iterations limit
    iterations.value = passes;
    bin = push_arr(bin, iterations.bytes);

    bin = push(bin, "\x48\xBA");            // mov rdx, iterations  ; Inner loop iterations limit
    iterations.value = area_size - 1;
    bin = push_arr(bin, iterations.bytes);

    bin = push(bin, "\x48\x31\xF6");        // xor rsi, rsi         ; Outer loop init

    bin = push(bin, "\x48\x31\xC9");        // xor rcx, rcx         ; Inner loop init

    bin = push(bin, "\x48\x03\x04\xCF");    // add rax, [rdi + rcx * 8]
    bin = push(bin, "\x48\xFF\xC1");        // inc rcx
    bin = push(bin, "\x48\x39\xD1");        // cmp rcx, rdx
    bin = push(bin, "\x75\xF4");            // jl inner_loop

    bin = push(bin, "\x48\xFF\xC6");        // inc rsi
    bin = push(bin, "\x4C\x39\xD6");        // cmp rsi, r10
    bin = push(bin, "\x75\xE9");            // jl outer_loop        ; Go to inner loop init

    bin = push(bin, "\xC3");                // ret

    measure_opcodes(bin_start, bin - bin_start, (uint64_t) area);

    printf("%s", test_description);

    for (size_t offset = 0; offset <= 8; ++offset)
    {
        printf("%.2lf  ", measure_opcodes(bin_start, bin - bin_start, (uint64_t) area + offset));
        fflush(stdout);
    }

    putchar('\n');

    munmap(area, area_size * sizeof(uint64_t));
}


void test_unaligned_read()
{
    puts("Unaligned sequential read\n");

    printf("    ");

    for (size_t offset = 0; offset <= 8; ++offset)
        printf("%6zu", offset);

    putchar('\n');

    test_unaligned_read_workload("L1:   ", 2048, 524588);
    test_unaligned_read_workload("L2:   ", 65536, 16384);
    test_unaligned_read_workload("L3:   ", 1048576, 1024);
    test_unaligned_read_workload("Mem:  ", 67108864, 16);

    putchar('\n');
}


int main()
{
    test_loop_alignment(false);
    test_loop_alignment(true);
    test_unaligned_read();

    return 0;
}
