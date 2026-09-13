.data
  savedFuncAddr dq 0
  savedRegsPtr dq 0
  savedRbp dq 0
  savedRsp dq 0

.code

EXTERN vmEnter : PROC
PUBLIC run_orig_code
PUBLIC vmEnter_stub

vmEnter_stub PROC
    ; pusing all regs
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8

    lea r15, [rsp + 80] 
    push r15 ; rsp

    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rcx, rsp
    mov rdx, [rsp + 128 + 8]
    sub rsp, 32
    ; todo: separate guest stack and vm stack
    call vmEnter
    add rsp, 32

    ; pop all regs

     pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp

    add rsp, 8
    
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    ret
vmEnter_stub ENDP

run_orig_code PROC
    jmp rcx
    
run_orig_code ENDP

getPeb PROC
    mov rax, gs:[60h]
    ret
getPeb ENDP

END
