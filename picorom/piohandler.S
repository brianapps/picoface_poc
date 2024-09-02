
#include "hardware/regs/addressmap.h"
#include "hardware/regs/pio.h"
#include "nmi.h"

.global piohandler

.extern rom_data
.extern rom_state

.section .time_critical.piohandler

.type piohandler,%function

r_rom_addr .req r4
r_pio_base .req r6
r_rom_state .req r5
r_nmi_rom_entry .req r7
r_nmi_exit .req r8
r_nmi_rom_exit .req r9
r_writable_from .req r10
r_rom_data .req r11
r_nmi_data .req r12


piohandler:

    ldr r1, =((0 << 14 | 0x66) << 17)
    mov r_nmi_rom_entry, r1               // r_nmi_rom_entry is shifted nmi entry address, this is M1 read at 0x66

    ldr r1, =((0 << 14 | EXITNMI) << 18)
    mov r_nmi_rom_exit, r1
                             
    
    ldr r_rom_state, =rom_state
    ldr r_pio_base, =PIO0_BASE
    mov r0, #1 
    mov r_nmi_exit, r0              // r_nmi_exit is the NMI exit address shift, 1 is an invalid value
    lsl r0, r0, #15
    mov r_writable_from, r0             // r_writable_from is writable address start and is initialised to 
                            // something outside the address range we handle.
    ldr r_rom_addr, =rom_data
    mov r_rom_data, r_rom_addr

    ldr r0, =(rom_data + 16384)
    mov r_nmi_data, r0

main_loop:
    ldr r1, =0x3FFF
check_fifo_empty:
    // The critical is the time is takes from when the pio sm puts an address
    // on the RX FIFO to time in which we put something on the TX FIFO of the
    // send data sm. We basically need to do this in about 18 cycles.
    // The code tries to avoid loads and stores to main memory because
    // these take two cycles. Accessing the pio state machines is via the SIO
    // so these ldr and str instructions take one cycles.

    // The next 4 instructions are basically a pio_sm_get_blocking(pio0, 0)
    // And this pulls 32 bits from the address/data fetcher pio state machine
    // This ends up with r0 holding
    //
    //  bits 0-13 : lower 14 bits of the spectrum address
    //  bit 15: Read flag = 1 for read this is effective the /WR line
    //  bit 16: /M1 flag = 0 for a M1 fetch 1 otherwise
    //  bits 17-24: the value on the data lines D0-D7 (useful only for writes)
    //  bits 25-31: random (this is stuff on the other gpio pins that we don't care about)

    ldr r0, [r_pio_base, #PIO_FSTAT_OFFSET]
    lsr r0, r0, #(PIO_FSTAT_RXEMPTY_LSB + 1)
    bcs check_fifo_empty
    ldr r0, [r_pio_base, #PIO_RXF0_OFFSET]   // r0 now holds data and address

    // Point A: Cycle count starts here

    and r1, r0, r1                   // r1 now holds just address
    lsl r2, r0, #17                  // carry flag is set for a read, and clear for a write
    bcc write_op
    ldr r3, [ r_rom_state, #0]       // r3 holds rom_state.flags
    lsr r3, r3, #1                   // CC is set if rom enable, zero flag is NMI check is enabled
    bcc read_rom_disabled
    beq send_data
    cmp r2, r_nmi_rom_entry
    beq activate_nmi
send_data:
    ldrb r0, [r_rom_addr, r1]
    // This is effectively a pio_sm_put(pio, 1, ) call
    str r0, [r_pio_base, #(PIO_TXF1_OFFSET) ]       
    // 12/13 cycles to complete this from Point A depending on which branch followed.

    // Check if have reached the exit point of the nmi routine
    // allow nmi to exit with either a M1 read or normal read of the exit address, hence the
    // shift
    lsl r2, r2, #1
    cmp r2, r_nmi_exit 
    beq exit_nmi

    
    // Just send anything non-zero to the putdata pio to indicate that we wish to hold
    // the /CSROM line and continue serving ROM data from the pico
    str r_rom_addr, [r_pio_base, #(PIO_TXF1_OFFSET) ]
    b main_loop

exit_nmi:
    // leaving nmi rom now
    
    ldr r0, [r_rom_state, #12]     // read rom_state.flags_on_nmi_exit
    // rom_state is zero to return to internal rom or 1 to serve up a custom ROM
    // Can can just pass this to the putdata pio because 0 means set the /CSROM low to 
    // enable the internal rom, anything else means hold the /CSROM line high.
    str r0, [r_pio_base, #(PIO_TXF1_OFFSET) ] 
    str r0, [r_rom_state, #0]      // restore the rom_state flags
    ldr r0, [r_rom_state, #8]      // read writableStartAddress in case we are now going to serving up a writable rom
    mov r_writable_from, r0        // and put into r_writable_from for later checks 
    mov r0, #1                     // set r_nmi_exit to an invalid shifted address, because (addr << 18) is never going to
                                   // equal 1. Therefore we have effectively disabled the nmi exit check from now on
    mov r_nmi_exit, r1                       
    mov r_rom_addr, r_rom_data     // Restore the rom address to the normal rom, regardless of whether the rom is enabled or not
                                   
    mov r0, #0                     // And set a variable to say we aren't serving the nmi rom anymore
    str r0, [r_rom_state, #4]    
    b main_loop

read_rom_disabled:
    beq main_loop          // zero flag is set if nmi check is not required
    cmp r2, r_nmi_rom_entry
    bne main_loop          // If we aren't accessing the nmi_entry address then loop back 
                           // otherwise activate the nmi rom

activate_nmi:
    mov r0, #0                                  // always serve a NOP as first NMI instruction, this saves a read instruction
                                                // but we need to ensure that nmi.asm also has a nop at location 0x66.
    str r0, [r_pio_base, #(PIO_TXF1_OFFSET) ]   // Send data to the PIO like an pio_sm_put() and this sends it off to the speccy.
                                                // 13 cycles to complete from point A.
    mov r_writable_from, r0                     // NMI always allows writes, so set allowable writes from 0 onwards
    mov r0, #1
    str r0, [r_pio_base, #(PIO_TXF1_OFFSET) ] 
    str r0, [r_rom_state, #4]            // nmi routine is now active
    str r0, [r_rom_state, #0]            // rom is enabled, because it is the nmi rom
    mov r_nmi_exit, r_nmi_rom_exit           // and set r_nmi_exit to actual exit address from the nmi rom
    mov r_rom_addr, r_nmi_data
    b main_loop
write_op:
    // r0 is data and address, r1 is just address
    // writes are allowed from the address in r_writable_from onwards
    cmp r1, r_writable_from
    blo main_loop
    lsr r0, r0, #16
    strb r0, [r_rom_addr, r1]
    b main_loop