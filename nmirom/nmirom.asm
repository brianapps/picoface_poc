    SLDOPT COMMENT WPMEM, LOGPOINT, ASSERTION

    DEVICE ZXSPECTRUM48

SCREEN: equ 0x4000
SCREEN_ATTRIB: equ 0x5800
BORDCR: equ  0x5C48

// The communication between the pico and the spectrum nmi rom is via special
// memory locations.

// Memory location zero is treated as special indicator:
// - When this value is 255, this lets the pico know there is work to do.
// - When work is complete, the pico sets to a value other than 255. 0 indicates
//   success, anything else an error of some sort



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


ACTION_SNA_SAVE equ 0x06
// WORD param1 = pointer to file name to save
// BYTE param2 = overwrite flag, force overwrite is non-zero
// status return is 0 for success, 1 for file exists, anything else is an error


ACTION_ROM_LIST equ 0x07
// as ACTION_SNA_LIST
// WORD param1  = start number
// WORD param2 = destination to write results
//  
// RESULTS are
//     BYTE countReturned  (will be no more than 10)
//     BYTE moreAviable  (non-zero if true)
//     WORD namepointer[countReturned]
//     String data follows


ACTION_ROM_CHANGE equ 0x08
// WORD param1 = pointer to file name to load
// status return is 0 for success, 1 for file exists, anything else is an error


STARTUP_ACTION_TRANSFER_SNAP equ 0x1
STARTUP_ACTION_LOAD_SNAP equ 0x2

STARTUP_ACTION_SAVE_MEMORY equ 0x3
STARTUP_ACTION_LOAD_MEMORY equ 0x4




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


nmientry:
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


.interruptsweredisbled:    
    ld hl, SCREEN
    ld de, screen_save
    ld bc, 6912
    ldir

    ld a, (start_up_action)
    cp ACTION_BEGIN_SNA_READ
    jr nz, 1F
    call sendsnapshottopico
    jr .exittopscreen

1:  call startmenu

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
self_modify_exit:
    nop
    nop
exitnmi:
    ret

iff2save:
    .byte 0

start_up_action: BYTE 0    
start_up_param1: WORD 0
start_up_param2: WORD 0

; -------------------------------------------
; Start up menu

startmenu:

.showtopscreen
    ld hl, .startmenu_def
    call showmenu
    ld hl, .startmenu_def
    jp menukeyhandler    

.handle_save:
    call savesnapshotscreen
    cp 0
    jr nz, .showtopscreen
    ret

.handle_exit:
    ret

.handle_load:
    call loadscreen
    jr .showtopscreen

.handle_change_rom:
    call changeromscreen
    jr .showtopscreen

.startmenu_def:
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




// HL points to menu info
showmenu:
    push hl
    call clearscreen
    pop hl

1:  
    ld a, (hl)
    cp 255   // 255 indicates end of menu items
    ret z
    // bit 7 set indicates to ignore this item
    bit 7, a
    jr z, 3F
    ld de, 7
    add hl, de
    jr 1B

3:  inc hl
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
    bit 7, a
    jr nz, .continuecheck
    cp c
    jr z, .foundmatch
.continuecheck
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

; ---------------------------------------------
;

testromdata:
    byte 10  ; number of items
    byte 1  ; are there more
    word .rom1, .rom2, .rom3, .rom4, .rom5, .rom6, .rom7, .rom8, .rom9, .rom10
.rom1 DZ "The first rom"
.rom2 DZ "The second"
.rom3 DZ "Third"
.rom4 DZ "AAD"
.rom5 DZ "EADSE"
.rom6 DZ "4th"
.rom7 DZ "4th"
.rom8 DZ "4th"
.rom9 DZ "4th"
.rom10 DZ "The stuff"


testromdata1:
    byte 3  ; number of items
    byte 0  ; are there more
    word .rom1, .rom2, .rom3, 0, 0, 0, 0, 0, 0, 0
.rom1 DZ "Page Two"
.rom2 DZ "Another one"
.rom3 DZ "And the final"



; hl points to list data
; de points to first menu item to update
; it expects the order to be previous, next, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0
updatemenulist:
    ld bc, 7
    ex hl, de
    ; check for previous 
    ld a, (ix + 2)
    or (ix + 3)
    jr z, .noprev
    res 7, (hl)
    jr 1F
.noprev
    set 7, (hl)
1:
    add hl, bc
    inc de
    ld a, (de)
    cp 0
    jr z, .nomore
    res 7, (hl)
    jr 1F
.nomore:
    set 7, (hl)
1:
    add hl, bc
    ex hl, de
    dec hl

    ld c, 255
    ld a, 10
    sub (hl)
    inc hl
    inc hl

    ld b, 10
1:
    cp b
    ex de, hl
    jr nc, .hideitem
    res 7, (hl)
    ex de, hl
    inc de
    inc de
    inc de
    ldi   
    ldi  ; ignore the decrement of bc because c is large enought that it doesn't affect b
    jr 2F
.hideitem
    set 7, (hl)
    ex de, hl
.advancetonext
    inc hl
    inc hl
    .5 inc de
2:
    inc de
    inc de
    djnz 1B

    ret





changeromscreen:
    ld ix, 0
    ld de, 0

.fetchlist
    ld (ix + 1), ACTION_ROM_LIST
    ld (ix + 2), de
    ld (.startpos), de
    ld hl, spare_space
    ld (ix + 4), hl
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B    


.updatefromdata
    ld de, .menuprev
    call updatemenulist

    ld hl, .menu
    call showmenu
.redisplay
    ld a, (.writable)
    cp 0
    jr z, 1F
    ld a, 'x'-' '
1:  add ' '
    ld c, a
2:  ld de, 0x0F0E
    call putchar


    ld hl, .menu
    jp menukeyhandler

.exithandler:
    ret

.nextkeyhandler:
    ld hl, (.startpos)
    ld de, 10
    add hl, de
    ex hl, de
    jp .fetchlist

.prevkeyhandler:
    ld hl, (.startpos)
    ld de, 10
    sub hl, de
    ex hl, de
    jp .fetchlist


.romhandlerzero
    ld a, 9
    jr 1F

.romhandler:
    sub '1'
1:
    ld b, 0
    ld c, a
    ld hl, spare_space + 2
    add hl, bc
    add hl, bc
    ld e, (hl)
    inc hl
    ld d, (hl)
    ex hl, de
    call changerom
    jr .redisplay

.writehandler:
    ld a, (.writable)
    xor 1
    ld (.writable), a
    jr .redisplay

.writable BYTE 0
.startpos WORD 0
.titlemsg DZ "Change Rom"
.snap0 BYTE 0
.exit DZ "Exit"
.next DZ "Next"
.prev DZ "Previous"
.writablemsg DZ "Writable [ ]"

.menu     
    MENU_ITEM 0, 1, 1, .titlemsg, 0
    MENU_ITEM 'w', 15, 2, .writablemsg, .writehandler
    MENU_ITEM 'x', 14, 2, .exit, .exithandler
.menuprev
    MENU_ITEM 'p', 14, 9, .prev, .prevkeyhandler
    MENU_ITEM 'n', 14, 20, .next, .nextkeyhandler
    MENU_ITEM '1', 3, 2, .snap0, .romhandler
    MENU_ITEM '2', 4, 2, .snap0, .romhandler
    MENU_ITEM '3', 5, 2, .snap0, .romhandler
    MENU_ITEM '4', 6, 2, .snap0, .romhandler
    MENU_ITEM '5', 7, 2, .snap0, .romhandler
    MENU_ITEM '6', 8, 2, .snap0, .romhandler
    MENU_ITEM '7', 9, 2, .snap0, .romhandler
    MENU_ITEM '8', 10, 2, .snap0, .romhandler
    MENU_ITEM '9', 11, 2, .snap0, .romhandler
    MENU_ITEM '0', 12, 2, .snap0, .romhandlerzero
    MENU_END


changerom:
    ld (ix + 2), hl
    ld (ix + 1), ACTION_ROM_CHANGE
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B

    cp 0
    ret nz

    // Use some self modifying code to jump to address zero
    // When the instruction has been fully read the pico knows
    // so disable the nmi rom and enter the new rom (or the internal one)
    // as address 0
    ld a, 0xC3 // JMP instruction
    ld (exitnmi - 2), a
    xor a
    ld (exitnmi - 1), a
    ld (exitnmi - 0), a
    jp exitnmi - 2



; -------------------------------------------
; Load snapshot menu




loadscreen:
    ld de, 0

.fetchlist
    ld ix, 0
    ld (ix + 1), ACTION_SNA_LIST
    ld (ix + 2), de
    ld (.startpos), de
    ld hl, spare_space
    ld (ix + 4), hl
    ld (ix + 0), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B


.updatefromdata
    ld de, .menuprev
    call updatemenulist
.showthemenu
    ld hl, .menu
    call showmenu
    ld hl, .menu
    jp menukeyhandler

.snaphandlerzero
    ld a, 9
    jr 1F

.snaphandler:
    sub '1'
1:
    ld b, 0
    ld c, a
    ld hl, spare_space + 2
    add hl, bc
    add hl, bc
    ld e, (hl)
    inc hl
    ld d, (hl)
    ex hl, de
    jp loadsnapshot

.nextkeyhandler:
    ld hl, (.startpos)
    ld de, 10
    add hl, de
    ex hl, de
    jp .fetchlist

.prevkeyhandler:
    ld hl, (.startpos)
    ld de, 10
    sub hl, de
    ex hl, de
    jp .fetchlist

.exithandler:    
    ret

.startpos WORD 0
.titlemsg DZ "Load snapshot"
.snap0 BYTE 0
.exit DZ "Exit"
.next DZ "Next"
.prev DZ "Previous"

.menu     
    MENU_ITEM 0, 1, 1, .titlemsg, 0
    MENU_ITEM 'x', 14, 2, .exit, .exithandler
.menuprev
    MENU_ITEM 'p', 14, 9, .prev, .prevkeyhandler
    MENU_ITEM 'n', 14, 20, .next, .nextkeyhandler
    MENU_ITEM '1', 3, 2, .snap0, .snaphandler
    MENU_ITEM '2', 4, 2, .snap0, .snaphandler
    MENU_ITEM '3', 5, 2, .snap0, .snaphandler
    MENU_ITEM '4', 6, 2, .snap0, .snaphandler
    MENU_ITEM '5', 7, 2, .snap0, .snaphandler
    MENU_ITEM '6', 8, 2, .snap0, .snaphandler
    MENU_ITEM '7', 9, 2, .snap0, .snaphandler
    MENU_ITEM '8', 10, 2, .snap0, .snaphandler
    MENU_ITEM '9', 11, 2, .snap0, .snaphandler
    MENU_ITEM '0', 12, 2, .snap0, .snaphandlerzero
    MENU_END



; -------------------------------------------
; Save snapshot menu

savesnapshotscreen:

    ld a, (sna_on_entry + SNAHEADER.BORD)
    ld (.bordercolour), a

    xor a
    ld (.namelen), a

.redisplay:
    call clearscreen
    ld de, 0x0101
    ld hl, .titlemsg
    call putstring

    ld de, 0x0301
    ld hl, .namelabel
    call putstring
    ld de, 0x0401
    ld b, 30
    ld a, 0b001111
    call putattributes

    ld de, 0x0601
    ld hl, .borderlabel
    call putstring

    ld de, 0x0901
    ld hl, .help1
    call putstring
    ld de, 0x0A01
    ld hl, .help2
    call putstring

    ld de, 0x0B01
    ld hl, .help3
    call putstring


    call .showbordername

    ld a, (.namelen)
    ld b, a
    ld de, 0x0401
    ld hl, .nametext
    cp 0
    jr z, 2F
1:  
    ld a, (hl)
    inc hl
    ld c, a
    call putchar
    djnz 1B
2:
    ld c, '_'
    call putchar


.waitfornextkey
    call waitforkeypress

    cp 0x18
    jr z, .exit

    cp 0x13
    jr z, .dosave

    cp 0x0A
    jr z, .backspace

    cp 31
    jr nc, .handlecharkey

    cp 0x9
    jr nc, .waitfornextkey

.handleborder
    and 0b111
    ld (.bordercolour), a
    call .showbordername
    jr .waitforkeyrelease    


.handlecharkey
    ld c, a
    ld a, (.namelen)
    cp 20
    jr nc, .waitforkeyrelease

    ld hl, .nametext
    ld d, 0
    ld e, a
    add hl, de
    ld (hl), c
    
    ld d, 4
    ld e, a
    inc e
    call putchar
    ld c, '_'
    call putchar

    inc a
    ld (.namelen), a

.waitforkeyrelease
    call waitforkeyrelease

1:   jr .waitfornextkey



.exit
    call waitforkeyrelease
    ld a, 1
    ret

.backspace
    ld a, (.namelen)
    cp 0
    jr z, .waitforkeyrelease
    dec a
    ld (.namelen), a
    ld d, 4
    inc a
    ld e, a
    ld c, '_'
    call putchar
    ld c, ' '
    call putchar
    jr .waitforkeyrelease


.dosave
    ld a, (.bordercolour)
    ld (sna_on_entry + SNAHEADER.BORD), a
    call sendsnapshottopico
    xor a ; don't overwrite in first instance
.saveit
    push ix
    ld ix, 0
    ld (ix + 4), a 
    ld (ix + 1), ACTION_SNA_SAVE
    ld a, (.namelen)
    ld e, a
    ld d, 0
    ld hl, .nametext
    ld (ix + 2), hl
    add hl, de
    ld (hl), 0
    ld (ix), 255
1:  ld a, (ix)
    cp 255
    jr z, 1B
    pop ix    

    cp 0
    ret z ; save was a success
    cp 1
    jr z, .prompttooverwrite

    ld a, 1
    ; some error
    ret

.prompttooverwrite:
    call overwritescreen
    jp nc, .redisplay ; don't overwrite, give the user a chance to change the name
    ld a, 1          ; otherwise have another go with a forced overwrite
    jr .saveit    

.showbordername:
    ld a, (.bordercolour)
    add a, a
    add a, a
    add a, a
    ld d, 0
    ld e, a
    ld hl, .border0
    add hl, de
    ld de, 0x0609
    call putstring
    ld a, (.bordercolour)
    rla
    rla
    rla
    and 0b111000
    cp 0b100000
    jr nc, .keepinkblack
    or 0b111
.keepinkblack
    ld de, 0x0609
    ld b, 7
    call putattributes    
    ret



.bordercolour DB 1
.nametext BLOCK 32
.namelen DB 1

.titlemsg DZ "Save Snapshot"
.help1 DZ "ENTER to save, BREAK To exit"
.help2 DZ "Shift 1-8 to change saved"
.help3 DZ "border colour"
.namelabel DZ "Name:"
.borderlabel DZ "Border: "
.border0 DZ "Black  "
.border1 DZ "Blue   "
.border2 DZ "Red    "
.border3 DZ "Magenta"
.border4 DZ "Green  "
.border5 DZ "Cyan   "
.border6 DZ "Yellow "
.border7 DZ "White  "


overwritescreen:
    ld hl, .menu_def
    call showmenu

    ld hl, .menu_def
    jp menukeyhandler

.handle_no:
    scf
    ccf
    ret

.handle_yes:
    scf
    ret

.menu_def:
    MENU_ITEM 0, 1, 1, .title1, 0
    MENU_ITEM 0, 2, 1, .title2, 0
    MENU_ITEM 'y', 4, 2, .yes, .handle_yes
    MENU_ITEM 'n', 5, 2, .no, .handle_no
    MENU_END
.title1 DZ "A file with this name exists"
.title2 DZ "Overwrite this file?"
.yes: DZ "Yes, overwrite this file"
.no: DZ "No, cancel"




; -------------------------------------------

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

sendsnapshottopico:
    push ix
    ld ix, 0
    call getimmode
    ld (sna_on_entry + SNAHEADER.IM), a
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

    ld a, (sna_header + SNAHEADER.BORD)
    and 7
    out (0xfe), a    

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
    ld c, a

    ld a, 0x3D
    ld i, a

    ld b, 0
    ei
    halt
    di
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
// b holds the count
// a the value to set

putattributes:
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

1:  ld (hl), a
    inc hl
    djnz 1B
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
    ld bc, 32 * 17 - 1
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
; shifted keys
keytable1:
    defb 'Z', 'X', 'C', 'V', '~'
    defb 'A', 'S', 'D', 'F','G'
    defb 'Q', 'W', 'E', 'R','T'
    defb 0x1, 0x2, 0x3, 0x4, 0x5
    defb 0xA, 0x9, 0x8, 0x7, 0x6
    defb 'P', 'O', 'I', 'U','Y'
    defb 0x13, 'L', 'K', 'J','H'
    defb 0x18, 'M', 'N', 'B', '~'

; symshifted keys
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

sna_header: BLOCK SNAHEADER

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


