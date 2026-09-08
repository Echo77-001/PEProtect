.data
  savedFuncAddr dq 0
  savedRegsPtr dq 0
  savedRbp dq 0
  savedRsp dq 0

.code

EXTERN globalCtx : QWORD
PUBLIC run_orig_code

run_orig_code PROC
    ;sub rsp, 48 ; shadow space
    ;and rsp, 0FFFFFFFFFFFFFFF0h
    jmp rcx
run_orig_code ENDP

getPeb PROC
    mov rax, gs:[60h]
    ret
getPeb ENDP

END
