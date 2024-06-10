    org 0x66

 ;   ld hl, 785
  ;  ld (hl), 12 
  ;  ret

    nop
    push af
    ld a, 2
    out (0xfe), a     
    pop af
exitnmi:
    retn