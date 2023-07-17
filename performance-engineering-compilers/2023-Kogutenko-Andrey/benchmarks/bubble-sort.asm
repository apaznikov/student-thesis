global bubble_sort

bubble_sort:

    mov r8, rsi         ; R8 - i

    align 64
    
    outer_loop:
    
        mov r9, 1       ; R9 - j
        
        inner_loop:
            
            mov edx, [rdi + r9 * 4]         ; arr[j]
            mov eax, [rdi + r9 * 4 - 4]     ; arr[j - 1]
            
            mov r10d, edx
            
            cmp eax, edx                    ; Comparing

            cmovg edx, eax                  ; Swapping
            cmovg eax, r10d
            
            mov [rdi + r9 * 4], edx         ; Push arr[j - 1] to arr[j]
            mov [rdi + r9 * 4 - 4], eax     ; Push arr[j] to arr[j - 1]
            
            inc r9              ; Increase j
            
            cmp r9, r8          ; Compare with top index
            jl inner_loop       ; If not equal, go to inner
                                ; loop beginning
        
        dec r8              ; Decrease top
        
        test r8, r8         ; Check if zero
        jnz outer_loop      ; Go to outer loop beginning
    
    ret
