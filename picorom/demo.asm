    org 34000

 ;   ld hl, 785
  ;  ld (hl), 12 
  ;  ret

    ld hl, 32768
    ld de, 32765
    ld bc, 1024
    ldir
    ret