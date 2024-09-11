## 3 Jun 2024

Managed to get something together to monitor reads and writes to ROM. However I was seeing odd writes to the ROM when running spectrum commands. 

e.g. POKE 16384, 255

was causing writes to ROM at address 0 and other commands were causing writes at other addresses maybe no higher that address 5.

By tracing the previous instruction fetch location, this pinpointed the 
[33C6: THE 'STACK LITERALS' SUBROUTINE](https://skoolkid.github.io/rom/asm/33C6.html). Particularly the instructions `LDIR` at 0x33E8 and `LD (DE),A` at 0x33E0.

Using Fuse and placing a breakpoint at this address, I can see that this routine geniunely writes to ROM addresses. This implies these writes are to be expected and the pico board write handling is working correctly.


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
issues. I can't get the JGH rom to work and dumping the gosh one has failed.

## 10 Jun 2024

I think the JGH rom switching was down to my hacky code I was using to modifying the copyright symbol
in the gosh rom. However timings are very tight.

Spent quite a while trying to optimise the rom handling routine and implemented something in assembler to ensure the pico responds quickly enough.

Spent an age this morning trying to trace down intermittent faults, e.g. pressing space causing crashes in the gosh rom. I retraced my steps without success until I realised the edge connector wasn't properly pushed in!

Enabled autopush on the address reading state machine to save a cycle.

Once the edge connector was firmly in place things moved forward well.

The optimised code gives about 4-5 cycles of breathing space.

Was able to add another flag to allow a rom to be writable.

Also implemented the first attempt of an nmi handler. It justs changes the border colour before returning. But it works and am feeling super chuffed. The RETN instruction is two bytes and therefore the exit address needs to be one byte further forward.


## 12 Jun 2024

I put in a lot of work implementing an simple NMI menu screen that saves the and restores the screen state.

All looked fine for a while until I stated seeing corruption when restoring the screen from the pico ram.

Dumping the contents of the picoram showed the screen had been correctly read from RAM and written to the pico. So the issue lied in the LDIR command that transferred the saved screen data back to the actual screen.

An awful lot of tweaking with timing went on and attempting to rewrite the PIOs and the code to improve timing etc. Then about 9:30pm last night I discovered that keeping the /CSROM line high (i.e. disabling the internal ROM) resolved the corruption. To test this, I first had to switch to the gosh rom and then use the nmi routines.

This result hinted that the /CSROM line timing was affecting pico rom reads during LDIR instructions so I played around with a ton of settings for hours.

In end trying something simple the following morning appears to have fixed things:

Adding a delay after pulling /CSROM high seems to work. I guess this gives the internal ROM a chance to fully disable before we take over the lines and plonk our data on it. Too early to say if this works, but it's feels good!

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

Not perfect have seen some corruption on knight lore after several attempts. Increasing the delay to 3 still doesn't completely fix it. 4 still causes corruption, and 5 causes the nmi to hang oh dear.

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


## 11 Jul 2024

A lot of work has gone on and a lot of problems have been encountered. Also was away for a week. I'll try and recall what has gone on.

Implemented a technique to save snapshot into ram and restore it later. I wasn't able to find a way to obtain the current border colour so this will have to be missing on saved snapshots.

Added lz4 frame compression on snapshot loading.

Got a PicoW and went about trying to implement a http server with file support. This was an utter pain. The built in lwip webserver is really designed to serve static web pages and not really suited for the REST style API I want to implement. For example there is no way to simply return a block of data to a GET request. Instead this needs to be served up as a file -- which can be crafted to use server side includes. This is a bit of a pain.

I then looked at spent a fair amount of time investigating https://gitlab.com/slimhazard/picow_http. This looked a lot more promising and has some quite extensive documentation. However after quite a bit of work, I hit problems attempting to POST more than 64KB of data. It seems that the entire design is pretty much hardcoded to use 16bit offsets and this is quite tricky to undo. In addition the library acculumates everything in buffers before invoking any callback, lwiP is limited to 64KB buffer so this means a redesign is required to allow big POSTs to be split into chunks that can be processed piecemeal.

Next I went onto roll my own HTTP server using raw lwIP TCP sockets. This was not fun. Wrote a very crude parser that processes HTTP headers without building up dynamically allocated buffers. Basically the system is designed to consume the entire lwIP buffer everytime something arrives. It was tricky to get things working but was able to implement a very basic REST API to store things in littleFS.

Performed a bit of restructing to the NMI ROM to make menu handling easier and implemented RPC message to list snapshots and load them by name. It's still very basic at the moment because I hit huge stability issues.

As soon as I started using the PicoW things went pear shaped. I'm not 100% convinced I've sorted them but they include:

- GAL chip not properly seated, which caused intermittent hangs.
- Wires becoming loose when I original the pico board was teased out and replaced with a picoW.
- Major instabilities I'm not 100% convinced that the WiFi is affecting things. Disabling WiFi improved things a bit but the system was not at all stable. Sometimes it would work but often snapshot loading would cause hangs and so on.

I felt the project had maybe reached the end until I started thinking about RAM contention. Afterall the system was absolutely rock solid for over an hour before I changed to the picow board.

Adding the line:

```
 bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_PROC1_BITS;
```

Made a world of difference. This gives CORE1 priority over CORE0 for memory and made the system work perfectly (I did 50 odd snapshot loads without error whereas I'd be lucky to get 10 before) with the WiFI disabled. However I did see issues with WIFI was re-enabled.

That said, when I retested the following day things seem to be solid with the WiFi enabled. That is apart from a bug in the NMI rom where the IX register wasn't being restored and would cause Underwurlde to crash when exiting the NMI without loading a snapshot. With these fixes, it seems to be back on track. However I'm reluctant to tempt fate.

Things to do include:
- Look out for a nicer font for the NMI menu
- Restore border colour in snapshot load
- Save snapshot to flash
- Support paging in snapshot screen
- Support loading snapshot 0 (this should be 10)
- Do some url decoding the the http server (so spaces can be handled)
