global _start

ARRAY_SIZE equ 100000000

section .bss

array resq ARRAY_SIZE
sum resq 1


section .text

_start:

    ; Initialize Xorshift64
    
    mov rax, 1

    ; Fill the array
    
    mov rcx, ARRAY_SIZE
    
    align 64
    
    fill:
    
        mov rbx, rax
        shl rax, 13
        xor rax, rbx
        
        mov rbx, rax
        shr rax, 7
        xor rax, rbx
        
        mov rbx, rax
        shl rax, 17
        xor rax, rbx
        
        mov [array + rcx * 8], rax
        loop fill
    
    ; Prepare to summarize
    
    xor rax, rax        ; Future sum of items
    
    ; Outer loop iterations
    
    mov rcx, 10
    
    lp:
    
        xor rdx, rdx        ; Items count
        
        align 64
        
        iterate:
            
            add rax, [array + rdx * 8]
            add rax, [array + rdx * 8 +  8]
            add rax, [array + rdx * 8 + 16]
            add rax, [array + rdx * 8 + 24]
        
        %ifdef INC
            inc rdx
            inc rdx
            inc rdx
            inc rdx
        %else
            add rdx, 4
        %endif
            
            cmp rdx, ARRAY_SIZE     ; Check if the end of array
            jne iterate             ; Next iteration
        
        loop lp

    ; Save sum to memory
    
    bswap rax           ; Change bytes order
    mov [sum], rax      ; Save to memory

    ; Print the result
    
    mov rax, 1          ; write() syscall index
    mov rdi, 1          ; stdout index
    mov rsi, sum        ; Set buffer address
    mov rdx, 8          ; Sum is 8 bytes length (64-bit register)
    syscall

    ; Terminate
    
    mov rax, 60         ; exit() syscall code
    xor rdi, rdi        ; Return code
    syscall
