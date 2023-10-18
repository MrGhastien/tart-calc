    GLOBAL memcpy, strlen, streq

memcpy:
    mov rax,rdi                 ;Return the address of the destination
    .loop:
    cmp rdx, 0                  ;Are there remaining bytes to copy ?
    jz .endloop                 ;If no, then get out of the loop
                                ;Else, copy one byte from source to destination
    mov cl, BYTE [rsi]
    mov [rdi], BYTE cl
    

    inc rdi                     ;Go to the next byte to copy
    inc rsi
    dec rdx
    jmp .loop                   ;start over !
    .endloop:

    ret

strlen:
    xor rax,rax

    .loop:
    cmp BYTE [rdi],0
    jz .endloop

    inc rax
    inc rdi
    jmp .loop
    .endloop: 
    ret
    
streq:
    ;; rdi : const char* a
    ;; rsi : const char* b
    .loop:
    mov al, [rdi]
    cmp BYTE al,[rsi]        ;*a == *b ?
    jnz .ret_false              ;if no then return false (0)
    cmp BYTE [rdi],0            ;*a == 0 ? (so we check *b == 0 too as *a == *b)
    jz .ret_true                ;if yes then end the loop,
    ;; we reached the end of the strings

    inc rdi                     ;Go to the next bytes and start over
    inc rsi
    jmp .loop
    
    .ret_true:
    mov rax, 1
    ret
    .ret_false:
    mov rax, 0
    ret
