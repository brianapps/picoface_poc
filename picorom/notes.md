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




