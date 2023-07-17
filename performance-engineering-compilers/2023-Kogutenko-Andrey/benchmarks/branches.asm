global _start

%ifndef ITEMS_COUNT
ITEMS_COUNT equ 100000000
%endif


section .bss

count resq 1


section .text

_start:

    ; Initialize Xorshift64
    
    mov rax, 1
    
    ; Prepare to counting
    
    xor rsi, rsi        ; Count of items
    mov rbx, 0          ; Threshold
    xor rcx, rcx        ; Items count
    
%ifdef BRANCHLESS

    ; CMOV does not work with constants - putting 1 to register
    mov rdi, 1
    
%endif

    ; Generate items and count
    
    mov rcx, ITEMS_COUNT
    
    align 64
    
    fill:
    
        ; Generate random value - Xorshift64 round
    
        ; x ^= x << 13
        mov rbx, rax
        shl rax, 13
        xor rax, rbx
        
        ; x ^= x >> 7
        mov rbx, rax
        shr rax, 7
        xor rax, rbx
        
        ; x ^= x << 17
        mov rbx, rax
        shl rax, 17
        xor rax, rbx
        
    %ifdef BRANCHLESS
        
        ; Branchless implementation - using CMOV
        
        xor rdx, rdx        ; Zero a value to add to count
        
        cmp rax, rbx        ; Compare generated value with threshold
        cmovg rdx, rdi      ; Set RDX to 1, if greater, than threshold
        
        add rsi, rdx        ; Add 0 or 1 to count
        
    %else
        
        ; Standard implementation
        
        cmp rax, rbx        ; Compare generated value with threshold
        jle continue        ; Skip increment, if less
        
        inc rsi             ; Increment count
    
        continue:
        
    %endif
        
        loop fill

    ; Save count to memory
    
    bswap rsi           ; Change bytes order
    mov [count], rsi    ; Save to memory

    ; Print the result
    
    mov rax, 1          ; write() syscall index
    mov rdi, 1          ; stdout index
    mov rsi, count      ; Set buffer address
    mov rdx, 8          ; Sum is 8 bytes length (64-bit register)
    syscall

    ; Terminate
    
    mov rax, 60         ; exit() syscall code
    xor rdi, rdi        ; Return code
    syscall
