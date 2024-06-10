    org 0x66

 ;   ld hl, 785
  ;  ld (hl), 12 
  ;  ret

    nop
    ld (oldsp), sp
    ld sp, 0x1000
    push af
    ld a, 4
    out (0xfe), a     
    pop af
    ld sp, (oldsp)
exitnmi:
    retn


oldsp:
    .defw 0