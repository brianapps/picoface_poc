    SLDOPT COMMENT WPMEM, LOGPOINT, ASSERTION

    DEVICE ZXSPECTRUM48

SCREEN: equ 0x4000
SCREEN_ATTRIB: equ 0x5800
BORDCR: equ  0x5C48

ACTION_BEGIN_SNA_READ: equ 0x01
// param1 = name pointer
// param2 = sna header offset


ACTION_SNA_READ_NEXT equ 0x02
// param1 = destination for data
// param2 = length to read
// returns, bytes read in param2

ACTION_SNA_BEGIN_WRITE equ 0x03
// param1 = destination of the sna header

ACTION_SNA_NEXT_WRITE equ 0x04
// param1 = source of data
// param2 = length to write


ACTION_SNA_LIST equ 0x05
// WORD param1  = start number
// WORD param2 = destination to write results
//  
// RESULTS are
//     BYTE countReturned  (will be no more than 10)
//     BYTE moreAviable  (non-zero if true)
//     WORD namepointer[countReturned]
//     String data follows


    MACRO MENU_ITEM key, y, x, msg, handler
        BYTE key
        BYTE y
        BYTE x
        WORD msg
        WORD handler
    ENDM

    MACRO MENU_END
        BYTE 255
    ENDM



    org 0x0

action: defb 0
action_params:

    org 0x38
im1handler:
    ld b, 1
    reti
    //jp startmenu

im2handler:
    ld b, 2
    reti

 

IM2HANDLER_SIZE: equ $ - im2handler 





TEST_AREA: equ 32768
PIXEL_SIZE: equ 192 * 32
    

startmenu:
    org 0x66
    nop

    ld (sna_on_entry + SNAHEADER.SP), SP
    ld (sna_on_entry + SNAHEADER.HL), HL
    ld (sna_on_entry + SNAHEADER.DE), DE
    ld (sna_on_entry + SNAHEADER.BC), BC
    ld (sna_on_entry + SNAHEADER.IX), IX
    ld (sna_on_entry + SNAHEADER.IY), IY
    ld sp, stack_top

    push af, bc, de, hl, ix

    push af
    pop hl
    ld (sna_on_entry + SNAHEADER.AF), HL
    exx
    ld (sna_on_entry + SNAHEADER.HLx), HL
    ld (sna_on_entry + SNAHEADER.DEx), DE
    ld (sna_on_entry + SNAHEADER.BCx), BC
    exx
    ex af, af'
    push af
    pop hl
    ld (sna_on_entry + SNAHEADER.AFx), HL
    ex af, af'

    ld a, (BORDCR)
    rra
    rra
    rra
    and 0x7
    ld (sna_on_entry + SNAHEADER.BORD), a    


    ld (sna_on_entry + SNAHEADER.IX), IX
    ld (sna_on_entry + SNAHEADER.IY), IY

    xor a
    ld (iff2save), a
    ld (sna_on_entry + SNAHEADER.IFF2), a

    ld a, i
    ld (sna_on_entry + SNAHEADER.I), a
    ;p/v contains iff2, p/v = 1 is even parity
    jp po,  .interruptsweredisbled

    ld a, 1
    ld (iff2save), a
    ld a, 0b100
    ld (sna_on_entry + SNAHEADER.IFF2), a

    call getimmode
    ld (immoddesave), a


.interruptsweredisbled:    
    ld hl, SCREEN
    ld de, screen_save
    ld bc, 6912
    ldir

.showtopscreen
    ld hl, startmenu_def
    call showmenu

    ld hl, startmenu_def
    jp menukeyhandler    

.handle_save:
    call savesnapshot
    jr .exittopscreen

.handle_exit:
    jr .exittopscreen   

.handle_load:
    call loadscreen
    jr .showtopscreen

.handle_change_rom:
    jr .showtopscreen

.exittopscreen

    ld hl, screen_save
    ld de, SCREEN
    ld bc, 6912
    ldir

    pop ix, hl, de, bc

    ld a, (iff2save)
    cp 0
    jr nz, .exitwithei
    pop af
    ld sp, (sna_on_entry + SNAHEADER.SP)
    jr exitnmi
.exitwithei:    
    pop af
    ld sp, (sna_on_entry + SNAHEADER.SP)
ei_exitnmi:
    ei
exitnmi:
    ret

oldborder:
    .byte 0
immoddesave:
    .byte 0
iff2save:
    .byte 0


startmenu_def:
    MENU_ITEM 0, 1, 1, .hellomsg, 0
    MENU_ITEM 's', 3, 2, .savemsg, startmenu.handle_save
    MENU_ITEM 'l', 4, 2, .loadmsg, startmenu.handle_load
    MENU_ITEM 'r', 5, 2, .chamgemsg, startmenu.handle_change_rom
    MENU_ITEM 'p', 6, 2, .pokemsg, 0
    MENU_ITEM 'x', 7, 2, .exitmsg,  startmenu.handle_exit
    MENU_END
.hellomsg DZ "picoFace"
.savemsg: DZ "Save snapshot"
.loadmsg: DZ "Load snapshot"
.chamgemsg: DZ "Change ROM"
.exitmsg: DZ "Exit"
.pokemsg: DZ "Poke"



sna_on_entry: BLOCK SNAHEADER


// a is key pressed
// c is key to check
// de is char to reverse
checkkey:
    cp a, c
    ret nz
    call reverseattr
    call waitforkeyrelease    
    cp a
    ret



// HL points to menu info
showmenu:
    push hl
    call clearscreen
    pop hl

1:  
    ld a, (hl)
    cp 255   // 255 indicates end of menu items
    ret z
    inc hl
    ld d, (hl)
    inc hl
    ld e, (hl)
    inc hl
    cp 0
    jr z, .dont_show_char
    // if key is lower case then show it as a capital
    cp 'a'
    jr c, 2F
    cp 'z' + 1
    jr nc, 2F
    add a, 'A' - 'a'
2:   
    ld c, a
    call putchar
    ld c, '-'
    call putchar

.dont_show_char:    
    ld c, (hl)
    inc hl
    ld b, (hl)
    inc hl
    inc hl
    inc hl

    ld a, b    // don't show message if NULL
    or c
    jr z, 1B


    push hl
    ld hl, bc
    call putstring
    pop hl
    jr 1B


// hl points to menu on entry
menukeyhandler:

    push hl
    call waitforkeypress
    pop hl
    push hl

    // c is now the key pressed
    ld c, a

.checkitems
    ld a, (hl)
    cp 255
    jr z, .checkdone
    cp c
    jr z, .foundmatch
    ld de, 7
    add hl, de
    jr .checkitems

.checkdone
    call waitforkeyrelease
    pop hl
    jr menukeyhandler


.foundmatch
    push bc    // we want to restore c because that holds the keypressed

    inc hl
    ld d, (hl)
    inc hl
    ld e, (hl)
    inc hl
    inc hl
    inc hl
    ld c, (hl)
    inc hl
    ld b, (hl)

    push bc
    push de
    call reverseattr
    call waitforkeyrelease
    pop de
    call reverseattr

    pop hl  // handler routine
    pop bc // c is the key pressed

    ld a, h
    or l
    jr z, .nohandler
    pop de // this is the menu pointer that we no longer need
    ld a, c
    jp (hl)

.nohandler
    pop hl
    jr menukeyhandler




.handler_addr WORD 0    






loadscreen:
    ld ix, 0
    ld (ix + 1), ACTION_SNA_LIST
    ld de, 0
    ld (ix + 2), de
    ld hl, spare_space
    ld (ix + 4), hl
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B

    ld a, (spare_space)
    ld b, a

    ld hl, spare_space + 2
    ld de, .menusnaps + 3
.updatemenustrings
    push bc

    ld bc, 2
    ldir

    ld bc, 5
    ex hl,de
    add hl, bc
    ex hl, de
    pop bc

    djnz .updatemenustrings

    ; ld hl, spare_space + 22
    ; ld (.menusnaps + 3), hl

    ld hl, .menu
    call showmenu
    ld hl, .menu
    jp menukeyhandler

.snaphandler:
    cp '0'
    jr z, .loadtenth
    sub '1'
    ld b, 0
    ld c, a
    ld hl, spare_space + 2
    add hl, bc
    add hl, bc
    ld e, (hl)
    inc hl
    ld d, (hl)
    ex hl, de

    ;ld hl, 0x25e1
    jp loadsnapshot




.loadtenth
    ret

.exithandler:    
    ret

.titlemsg DZ "Load snapshot"
.snap0 BYTE 0

.exit DZ "Exit"
.next DZ "Next"
.prev DZ "Previous"

.menu     

    MENU_ITEM 0, 1, 1, .titlemsg, 0

    MENU_ITEM 'x', 14, 2, .exit, .exithandler
    MENU_ITEM 'n', 14, 9, .next, 0
    MENU_ITEM 'p', 14, 16, .prev, 0
.menusnaps:
    MENU_ITEM '1', 3, 2, .snap0, .snaphandler
    MENU_ITEM '2', 4, 2, .snap0, .snaphandler
    MENU_ITEM '3', 5, 2, .snap0, .snaphandler
    MENU_ITEM '4', 6, 2, .snap0, .snaphandler
    MENU_ITEM '5', 7, 2, .snap0, .snaphandler
    MENU_ITEM '6', 8, 2, .snap0, .snaphandler
    MENU_ITEM '7', 9, 2, .snap0, .snaphandler
    MENU_ITEM '8', 10, 2, .snap0, .snaphandler
    MENU_ITEM '9', 11, 2, .snap0, .snaphandler
    MENU_ITEM '0', 12, 2, .snap0, .snaphandler
    MENU_END



loadscreenold:
   call clearscreen

    ld de, 0x0101
    ld hl, .titlemsg
    call putstring

    ld de, 0x0302
    ld hl, .snap1
    call putstring

    ld de, 0x0402
    ld hl, .snap2
    call putstring

    ld de, 0x0502
    ld hl, .snap3
    call putstring    

    ld de, 0x0602
    ld hl, .exitmsg
    call putstring


.keyhandling
    call waitforkeypress



    cp '1'
    jr nz, 1F
    ld de, 0x0302
    call reverseattr
    call waitforkeyrelease
    xor a
    jp loadsnapshot

1:  cp '2'
    jr nz, 1F
    ld de, 0x0402
    call reverseattr
    call waitforkeyrelease
    ld a, 1
    jp loadsnapshot


1:  cp '3'
    jr nz, 1F
    ld de, 0x0502
    call reverseattr
    call waitforkeyrelease
    ld a, 255
    jp loadsnapshot    

1:  cp 'x'
    jr nz, 1F
    ld de, 0x0602
    call reverseattr
    call waitforkeyrelease
    ret

1:
    call waitforkeyrelease
    jr .keyhandling
    ret

.titlemsg DZ "Load snapshotx"
.snap1 DZ "1-Manic Miner"
.snap2 DZ "2-Knight Lore"
.snap3 DZ "3-Internal snapshot"
.exitmsg DZ "X-Exit"

     STRUCT SNAHEADER
I    BYTE
HLx  WORD
DEx  WORD
BCx  WORD
AFx  WORD
HL   WORD
DE   WORD
BC   WORD
IY   WORD
IX   WORD
IFF2 BYTE
R    BYTE
AF   WORD
SP   WORD
IM   BYTE
BORD BYTE
     ENDS

savesnapshot:
    push ix
    ld ix, 0
    ld hl, sna_on_entry
    ld (ix + 2), hl
    ld (ix + 1), ACTION_SNA_BEGIN_WRITE
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B


    ld de, screen_save
    ld bc, 6912
    ld (ix + 2), de
    ld (ix + 4), bc
    ld (ix + 1), ACTION_SNA_NEXT_WRITE
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B

    ld b, 12
    ld hl, 16384 + 6912

.transfer
    push bc
    ld de, spare_space
    ld bc, 3520
    ld (ix + 2), de
    ld (ix + 4), bc
    ld (ix + 1), ACTION_SNA_NEXT_WRITE
    ldir
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B
    pop bc
    djnz .transfer
    pop ix

    ret


// enter with hl pointing to the snapshot name string
loadsnapshot:
    push ix
    ld ix, 0
    ld (ix + 2), hl
    ld hl, sna_header
    ld (ix + 4), hl
    ld (ix + 1), ACTION_BEGIN_SNA_READ
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B

    ld b, 8
    

    ld de, 16384

.transfer
    push bc
    ld hl, screen_save
    ld bc, 1024 * 6
    ld (ix + 2), hl
    ld (ix + 4), bc
    ld (ix + 1), ACTION_SNA_READ_NEXT
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B

    ldir

    pop bc
    djnz .transfer
    pop ix

    ld a, (sna_header + SNAHEADER.I)
    ld i, a
    ld a, (sna_header + SNAHEADER.IM)
    cp 2
    jr z, .im2mode
    im 1
    jr 1F
.im2mode
    im 2
1:
    exx
    ex af, af'
    ld hl, (sna_header + SNAHEADER.AFx)
    push hl
    pop af
    ld hl, (sna_header + SNAHEADER.HLx)
    ld de, (sna_header + SNAHEADER.DEx)
    ld bc, (sna_header + SNAHEADER.BCx)
    exx
    ex af, af'
    ld hl, (sna_header + SNAHEADER.AF)
    push hl
    ld hl, (sna_header + SNAHEADER.HL)
    ld de, (sna_header + SNAHEADER.DE)
    ld bc, (sna_header + SNAHEADER.BC)
    ld iy, (sna_header + SNAHEADER.IY)
    ld ix, (sna_header + SNAHEADER.IX)
    ld a, (sna_header + SNAHEADER.IFF2)

    bit 2, a

    jr z, .di

    pop af
    ld sp, (sna_header + SNAHEADER.SP)
    jp ei_exitnmi


.di:
    pop af
    ld sp, (sna_header + SNAHEADER.SP)
    di
    jp exitnmi

   
getimmode:
    ld de, 0x3E3E
    ld hl, im2handler
    ld bc, IM2HANDLER_SIZE
    ldir

    ld hl, 0x3D00
    ld a, 0x3E
    ld (hl), a
    ld de, 0x3D01
    ld bc, 256
    ldir

    ld a, i
    //push af

    ld c, a

    ld a, 0x3D
    ld i, a

    ei
    halt
    di


    //pop af
    //ld i, a

    ld a, c
    ld i, a
    ld a, b    
    ret


; Pauses for a while.
; de: wait time, ca. de*0.1ms
pause:
	push af
	push bc

pause_loop_l2:
	ld b,26
pause_loop_l1:
	djnz pause_loop_l1 ; 1 cycle should be roughly 100us=0.1ms

	dec de
	ld a,d
	or e
	jr nz,pause_loop_l2

	pop bc
	pop af
	ret


// de location (y, x) and hl points to string
putstring:
   
1   ld a, (hl)
    or a
    ret z
    ld c, a
    call putchar
    inc hl
    jr 1B


putchar:
    push af, de, bc, hl

    ld h, 0
    ld l, c
    add hl, hl
    add hl, hl
    add hl, hl
    ld bc, charbank - 32 * 8
    add hl, bc

    // de holds y/x character position
    // 
    ld a, d
    and 0b111
    rrca
    rrca
    rrca
    or e
    ld e, a

    ld a, d
    and 0b00011000
    or  0b01000000
    ld d, a

    ld b, 8

1   ld a, (hl)
    ld (de), a
    inc hl
    inc d
    djnz 1B

    pop hl, bc, de, af
    inc e
    ret


// de holds the y, x position to reverse
reverseattr:
    ld hl, SCREEN_ATTRIB
    ld l, e

    ld e, 0
    srl d
    rr e
    srl d
    rr e
    srl d
    rr e

    add hl, de

    ld a, (hl)
    and a, 0b111
    sla a
    sla a
    sla a
    ld e, a
    ld a, (hl)
    and a, 0b111000
    srl a
    srl a
    srl a
    or e
    ld e, a
    ld a, (hl)
    and 0b11000000
    or e

    ld (hl), a


    ret




clearscreen:
    ld hl, SCREEN_ATTRIB
    ld a, 0b000111  // black paper, white ink
    ld (hl), a
    ld de, SCREEN_ATTRIB + 1
    ld bc, 32 * 16 - 1
    ldir

    xor a
    ld hl, SCREEN
    ld (hl),a
    ld de, SCREEN + 1
    ld bc, 32 * 16 * 8 - 1   // clear 16 rows of 32 characters each that is 8 pixels high
    ldir
    ret


waitforkeypress:
    call getkeypress
    cp 0
    jr z, waitforkeypress
    cp 255
    jr z, waitforkeypress
    ret

waitforkeyrelease:
    ld de, 1000
    call pause
    call getkeypress
    cp 0
    jr nz, waitforkeyrelease
    ld de, 100
    call pause
    ret
    

// FEFF Shift, Z, X, C, V
// FDFF A, S, D, F, G
// FBFF Q W E R T
// F7FF 1 2 3 4 5
// EFFF 0 9 8 7 6
// DFFF P O I U Y
// BFFF /n L K J H
// 7FFF spc ss M N B

getkeypress:
    ld h, 0 // shift code
    ld l, 0 // key index press
    ld e, 0 // current key index

    ld b, 0xFE


.scanloop
    ld c, 0xFE

    in a, (c)

    bit 0, b
    jr nz, .checksymshift
    rrca
    jr c, .countbits
    ld h, 1
    jr .countbits

.checksymshift
    bit 7, b
    jr nz, .countbits
    rra
    rra
    jr c, .symshiftnotpressed
.symshiftpressed
    set 1, h
.symshiftnotpressed
    rlca

.countbits:
    push bc
    ld b, 5
1:
    inc e
    rra
    jr c, .nokey

    ld c, a

    xor a
    cp l
    jr z, .firstkeypress

.secondkeypress
    pop bc
    ld a, 0xFF
    ret


.firstkeypress
    ld a, c
    ld l, e
.nokey
    djnz 1B

    pop bc
    scf
    rl b
    jr c, .scanloop


    xor a
    cp l
    ret z


    bit 0, h
    jr z, 1F
    ld de, keytable1 - 1
    jr 3F
1:
    bit 1, h
    jr z, 1F
    ld de, keytable2 - 1
    jr 3F
1:
    ld de, keytable0 - 1
3:
    ld h, 0
    add hl, de
    ld a, (hl)
    ret

keytable0:
    defb 'z', 'x', 'c', 'v', '~'
    defb 'a', 's', 'd', 'f','g'
    defb 'q', 'w', 'e', 'r','t'
    defb '1', '2', '3', '4','5'
    defb '0', '9', '8', '7','6'
    defb 'p', 'o', 'i', 'u','y'
    defb 0x13, 'l', 'k', 'j','h'
    defb ' ', 'm', 'n', 'b', '~'
keytable1:
    defb 'Z', 'X', 'C', 'V', '~'
    defb 'A', 'S', 'D', 'F','G'
    defb 'Q', 'W', 'E', 'R','T'
    defb '1', '2', '3', '4','5'
    defb '0', '9', '8', '7','6'
    defb 'P', 'O', 'I', 'U','Y'
    defb 0x13, 'L', 'K', 'J','H'
    defb 0x10, 'M', 'N', 'B', '~'

keytable2:
    defb ':', 0x60, '?', '/', '~'
    defb '~', '|', '\', '{','}'
    defb '~', '~', '~', '<','>'
    defb '!', '@', '#', '$','%'
    defb '_', ')', '(', "\'", '&'
    defb '"', ';', '~', ']','['
    defb 0x13, '=', '+', '-','^'
    defb ' ', '.', ',', '*', '~'    

charbank:
    defb 0,0,0,0,0,0,0,0
    defb 24,24,24,24,24,0,24,0
    defb 108,108,108,0,0,0,0,0
    defb 54,54,127,54,127,54,54,0
    defb 12,63,104,62,11,126,24,0
    defb 96,102,12,24,48,102,6,0
    defb 56,108,108,56,109,102,59,0
    defb 12,24,48,0,0,0,0,0
    defb 12,24,48,48,48,24,12,0
    defb 48,24,12,12,12,24,48,0
    defb 0,24,126,60,126,24,0,0
    defb 0,24,24,126,24,24,0,0
    defb 0,0,0,0,0,12,12,48
    defb 0,0,0,126,0,0,0,0
    defb 0,0,0,0,0,0,24,24
    defb 0,6,12,24,48,96,0,0
    defb 60,102,110,126,118,102,60,0
    defb 24,56,24,24,24,24,126,0
    defb 60,102,6,12,24,48,126,0
    defb 60,102,6,28,6,102,60,0
    defb 12,28,60,108,126,12,12,0
    defb 126,96,124,6,6,102,60,0
    defb 28,48,96,124,102,102,60,0
    defb 126,6,12,24,48,48,48,0
    defb 60,102,102,60,102,102,60,0
    defb 60,102,102,62,6,12,56,0
    defb 0,0,24,24,0,24,24,0
    defb 0,0,24,24,0,24,24,48
    defb 12,24,48,96,48,24,12,0
    defb 0,0,126,0,126,0,0,0
    defb 48,24,12,6,12,24,48,0
    defb 60,102,12,24,24,0,24,0
    defb 60,102,110,106,110,96,60,0
    defb 60,102,102,126,102,102,102,0
    defb 124,102,102,124,102,102,124,0
    defb 60,102,96,96,96,102,60,0
    defb 120,108,102,102,102,108,120,0
    defb 126,96,96,124,96,96,126,0
    defb 126,96,96,124,96,96,96,0
    defb 60,102,96,110,102,102,60,0
    defb 102,102,102,126,102,102,102,0
    defb 126,24,24,24,24,24,126,0
    defb 62,12,12,12,12,108,56,0
    defb 102,108,120,112,120,108,102,0
    defb 96,96,96,96,96,96,126,0
    defb 99,119,127,107,107,99,99,0
    defb 102,102,118,126,110,102,102,0
    defb 60,102,102,102,102,102,60,0
    defb 124,102,102,124,96,96,96,0
    defb 60,102,102,102,106,108,54,0
    defb 124,102,102,124,108,102,102,0
    defb 60,102,96,60,6,102,60,0
    defb 126,24,24,24,24,24,24,0
    defb 102,102,102,102,102,102,60,0
    defb 102,102,102,102,102,60,24,0
    defb 99,99,107,107,127,119,99,0
    defb 102,102,60,24,60,102,102,0
    defb 102,102,102,60,24,24,24,0
    defb 126,6,12,24,48,96,126,0
    defb 124,96,96,96,96,96,124,0
    defb 0,96,48,24,12,6,0,0
    defb 62,6,6,6,6,6,62,0
    defb 24,60,102,66,0,0,0,0
    defb 0,0,0,0,0,0,0,255
    defb 28,54,48,124,48,48,126,0
    defb 0,0,60,6,62,102,62,0
    defb 96,96,124,102,102,102,124,0
    defb 0,0,60,102,96,102,60,0
    defb 6,6,62,102,102,102,62,0
    defb 0,0,60,102,126,96,60,0
    defb 28,48,48,124,48,48,48,0
    defb 0,0,62,102,102,62,6,60
    defb 96,96,124,102,102,102,102,0
    defb 24,0,56,24,24,24,60,0
    defb 24,0,56,24,24,24,24,112
    defb 96,96,102,108,120,108,102,0
    defb 56,24,24,24,24,24,60,0
    defb 0,0,54,127,107,107,99,0
    defb 0,0,124,102,102,102,102,0
    defb 0,0,60,102,102,102,60,0
    defb 0,0,124,102,102,124,96,96
    defb 0,0,62,102,102,62,6,7
    defb 0,0,108,118,96,96,96,0
    defb 0,0,62,96,60,6,124,0
    defb 48,48,124,48,48,48,28,0
    defb 0,0,102,102,102,102,62,0
    defb 0,0,102,102,102,60,24,0
    defb 0,0,99,107,107,127,54,0
    defb 0,0,102,60,24,60,102,0
    defb 0,0,102,102,102,62,6,60
    defb 0,0,126,12,24,48,126,0
    defb 12,24,24,112,24,24,12,0
    defb 24,24,24,24,24,24,24,0
    defb 48,24,24,14,24,24,48,0
    defb 49,107,70,0,0,0,0,0
    defb 60,66,153,161,161,153,66,60


; manicscr:

;     INCBIN "../picorom/data.scr"


    SAVEBIN "nmirom.bin", 0x0, $
    SAVEBIN "fuserom.bin", 0x0, 0x4000





; Stack: this area is reserved for the stack
STACK_SIZE: equ 150    ; in words


; Reserve stack space
    defw 0  ; WPMEM, 2
stack_bottom:
    defs    STACK_SIZE*2, 0
stack_top:
    ;defw 0
    defw 0  ; WPMEM, 2    


screen_save: BLOCK 6912

sna_header: BLOCK 27

spare_size: equ 0x4000 - $

spare_space: BLOCK spare_space
    


; typing_demo:
;  ld de, 0x0700
;     ld c, '_'
;     call putchar

;     push de
; .readkeys
;     call getkeypress
;     cp 0
;     jr z, .readkeys
;     pop de
;     push de
;     ld c, a
;     call putchar
;     pop de
;     inc e
;     res 5, e
;     push de
;     ld c, '_'
;     call putchar

; .waitforrelease
;     call getkeypress
;     cp 0
;     jr nz, .waitforrelease
;     jr .readkeys


