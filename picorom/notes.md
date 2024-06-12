## 3 Jun 2024

I was seeing what I thought were writes to ROM when I was running spectrum commands.

e.g. POKE 16384, 255

was causing writes to ROM at address 0 and other commands were causing writes as other addresses maybe no higher that address 5.

By tracing the previous instruction fetch location, this pinpointed the 
[33C6: THE 'STACK LITERALS' SUBROUTINE](https://skoolkid.github.io/rom/asm/33C6.html). Particular the instructions `LDIR` at 0x33E8 and `LD (DE),A` at 0x33E0.

Using Fuse and placing a breakpoint at this address, I can see that this routine geniunely writes to ROM addresses. This implies the pico board write handling is working correctly.


## 4 Jun 2024

Have got the rom peek working but not sure how.


```
.program fetchaddr
.side_set 1
.wrap_target
    WAIT 0 GPIO PIN_PICOREQ SIDE 1 
    IN PINS, 7 SIDE 0 [5]
    IN PINS, 25 SIDE 0 
    PUSH SIDE 1
    WAIT 1 GPIO PIN_PICOREQ SIDE 1 [1]
.wrap

.program putdata
.wrap_target
    PULL
    OUT PINS, 8
    MOV OSR, ~NULL
    OUT PINDIRS, 8
    WAIT 1 GPIO PIN_PICOREQ [15]
    MOV OSR, NULL
    OUT PINDIRS, 8
.wrap


; disable the rom for one memory fetch
.program disablerom
.wrap_target
    PULL 
    SET PINS, 1
    WAIT 1 GPIO PIN_PICOREQ [15]
    SET PINS, 0
.wrap
```

And then in code:

```
        if (access == 3 && romaddr == 66) {
            pio_sm_put(pio, SM_CSROM, 0);
            pio_sm_put(pio, SM_OUTDATA, c);
            c++;
        }
```

The first PEEK 66 gave 181 but the next gave 2 and so on.


By doing this:

```
    MOV OSR, ~NULL
    OUT PINDIRS, 8
    MOV OSR, NULL
    OUT PINDIRS, 8
    SET PINS, 0    
```

The first peek works, so let's narrow it down.

This also worked:
```
    MOV OSR, NULL
    OUT PINDIRS, 8
    SET PINS, 0    
```

OK we need this:

```
.program fetchaddr
.side_set 1
    WAIT 0 GPIO PIN_PICOREQ SIDE 1 [15]
    WAIT 1 GPIO PIN_PICOREQ SIDE 1 [15]
.wrap_target
    WAIT 0 GPIO PIN_PICOREQ SIDE 1 
    IN PINS, 7 SIDE 0 [10]
    IN PINS, 25 SIDE 0 
    PUSH SIDE 1
    WAIT 1 GPIO PIN_PICOREQ SIDE 1 [1]
.wrap


.program putdata
    SET PINS, 0    
.wrap_target
    PULL
    SET PINS, 1
    OUT PINS, 8
    MOV OSR, ~NULL
    OUT PINDIRS, 8
    WAIT 1 GPIO PIN_PICOREQ [3]
    MOV OSR, NULL
    OUT PINDIRS, 8
    SET PINS, 0
.wrap
```

And can remove the disable rom program and configure the set pins for putdata to be
the romcs line.

## 5 Jun 2024

Issues with the rom interception only working 2nd time around persisted despite tweaking PIO
routines. In the end I tried `cmake -DPICO_NO_FLASH=1` and that seemed to fix it.

It would appear that some loading/running from flash was interfering. In the end just making
the key function run from RAM fixed things. e.g. with

```
void __time_critical_func(do_my_pio)() {
    ...
}
```

## 6 Jun 2024

Managed to get the pico to perform ROM switching. Though things aren't without
issues. I can't get the FGH rom to work and dumping the gosh one has failed.

## 10 Jun 2024

I think the FGH rom switching was down to me modifying the copyright symbol
in the gosh rom. However timings are very tight.

Spent quite a while trying to optimise the rom handling routine and implemented something in assembler to ensure the pico responds quickly enough.

Spent an age this morning trying to trace down intermittent faults, e.g. press space causing crashes in the gosh rom. I retraced my steps without success until I realised the edge connector wasn't properly pulled in!

Enabled autopush on the address reading state machine to save a cycle.

Once the edge connector was firmly in place things moved forward well.

The optimised code gives about 4-5 cycles of breathing space.

Was able to add another flag to allow a rom to be writable.

Also implemented the first implementation of an nmi handler. It justs changes the border colour before returning. But it works and am feeling super chuffed. The RETN instruction is two bytes and therefore the exit address needs to be one byte further forward.


## 12 Jun 2024

I put in a lot of work implementing an simple NMI menu screen that saves the and restores the screen state.

All looked fine for a while until I stated seeing corruption when restoring the screen from the pico ram.

Dumping the contents of the picoram showed the screen had been correctly read from RAM and written to the pico. So the issue lied in the LDIR command that transferred the saved screen data back to the actual screen.

An awful lot of tweaking with timing went on and attempting to rewrite the PIOs and the code to improve timing etc. Then about 9:30pm last night I discovered that keeping the /CSROM line high (i.e. disabling the internal ROM) resolved the corruption. To test this, I first had to switch to the gosh rom and then use the nmi routines.

This result hinted that the /CSROM line timing was affecting pico rom reads during LDIR instructions so I played around with a ton of settings for hours.

In end trying something simple the following morning appears to have fixed things:

Adding a delay after pulling /CSROM high seems to work. I guess this gives the internal ROM a chance to fully disable before we take over the
lines an plonk our data on it. Too early to say if this works, but it's feels good!

```
.program putdata
.side_set 1
    PULL SIDE 0
.wrap_target
    OUT PINS, 8 SIDE 1 [2]
    MOV OSR, ~NULL SIDE 1
    OUT PINDIRS, 8 SIDE 1
    WAIT 1 GPIO PIN_PICOREQ SIDE 1
    MOV OSR, NULL SIDE 0
    OUT PINDIRS, 8 SIDE 0
    PULL SIDE 0
.wrap
```

Not perfect have seen some corruption on knight lore after several attempts. Increasing to delay to 3 still doesn't completely fix it.
4 still causes corruption, and 5 causes the nmi to hang oh dear.

Well, when I went back and tested keeping the rom disabled there was still corruption. So something else is happening.

Right, the corruption in Knight Lore is probably because the return address is pushed onto the stack during NMI handling. Knight Lore is probably making clever use of the stack pointer to shuffle sprites into the screen. This would explain why the corruption is permanent. In addition I've written a soak test to ldir into the pico ram, then ldir from the pico ram to the screen and compare. After 49mins there have been no errors so I'd say it's looking good. (it's done 1 hour.)

Now it's time to think about the design of the speccy to pico RPC mechanism.

Functions to include:

open_sna_for_write: name -> success/fail
write_to_sna: source, length -> success/fail
close_sna -> success/fail

open_sna_for_read: name -> success/fail
read_from_sna: destination, length -> success/fail, bytes read
close_sna -> success/fail


list_snapshots: start_index, buffer, buffer_length -> success/fail, entries read, total count



