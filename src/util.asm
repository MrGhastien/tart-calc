    GLOBAL memcpy, strlen, streq

memcpy:
    push rbx
    add rsi,rdx                 ; Add byte_count to src
    add rdx,rdi                 ; add byte_count to dst
    ;;  Use rdx instead of rdi because we want to
    ;; keep the beginning of the destination

    .loop:
    cmp rdx,rdi                 ;Did we arrive at the beginning of the destination ?
    jz .endloop                 ;If yes get out of the loop
                                ;Else, copy one byte from source to destination
    mov BYTE cl,[rsi]
    mov [rdx],cl
    

    dec rdx                     ;Go to the next byte to copy
    dec rsi
    jmp .loop                   ;start over !
    .endloop:

    mov rdi,rax                 ;Return the address of the destination
    pop rbx
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
    cmp BYTE [rdi],[rsi]        ;*a == *b ?
    jnz .ret-false              ;if no then return false (0)
    cmp BYTE [rdi],0            ;*a == 0 ? (so we check *b == 0 too as *a == *b)
    jz .ret-true                ;if yes then end the loop,
    ;; we reached the end of the strings

    inc rdi                     ;Go to the next bytes and start over
    inc rsi
    jmp .loop
    
    .ret_true:
    mov 1,rax
    ret
    .ret_false:
    mov 0,rax
    ret
