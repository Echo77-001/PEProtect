.data
  sig_start dq 0DEADC0DE11223344h
  sig_end   dq 0DEADC0DE44332211h

.code

PUBLIC __peprotect_start
PUBLIC __peprotect_end

__peprotect_start PROC
    lea rax, [sig_start]
    ret
__peprotect_start ENDP

__peprotect_end PROC
    lea rax, [sig_end]
    ret
__peprotect_end ENDP

END