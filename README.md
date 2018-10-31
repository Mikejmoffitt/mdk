Mega Drive Toolchain
===================

This project aims to deliver some core support toolchain components used in developing software for the Sega Genesis / Mega Drive. Extant solutions exist on both ends of the complexity spectrum, with straight 68000 assembly programming on one and SGDK on the other. Both are great solutions, but it's never a bad time for some not-invented-here syndrome to come through and help deliver a third option somewhere in the middle.

This project is oriented towards development on a Linux or Unix-like host, and is C-focused. However, as needed, some functionality can be implemented in 68000 assembly. Presently, this is limited to the C runtime startup, and IRQ handlers.

Rather than provide a library to link against, this project is intended to act as a skeleton that is the basis of another project. MD support functions may be compiled and used.

A makefile is provided that will recursively look for source files in the src/ directory. It links using the provided linker script, which puts ROM from $000000 to $3FFFFF and RAM at $FF0000-$FFFFFF.
The crt0 startup code does nothing more than satisfy TMSS, clear main memory, initialize variables, set the stack, and move forwards.

Setup
=====

Prerequisites
-------------
This project uses the m68k-elf-gcc compiler suite. The easiest way to get it installed is probably to install a Gendev release, or build it yourself. All that is needed is m68k-elf-gcc and friends (as, ld, etc). You may modify the Makefile to point GBIN to wherever this lives. Despite this suggestion, no other parts of SGDK or Gendev are relied upon.

Getting Started
---------------
A simple example program is included in src/main.c, but generally everything needed is in src/md. src/util contains some potentially useful more exotic functions that I do not consider core functionality.
Within src/md are several Mega Drive components. For example, vdp.h and vdp.c contain functions related to the Mega Drive's Video Display Processor. For each component, the header files are fairly descriptive.
However, you do not need to include each component to use it. You may simply use

`#include "md/megadrive.h"`

and initialize the Mega Drive to sane defaults using

`megadrive_init();`

Afterwards, an infinite loop like this is sufficient:

```
while(1)
{
	// Do whatever you want here - draw some text, move some sprites around, run game logic
	megadrive_finish();
}
```

Finally,

`make`

Components
==========
For each component, I recommend reading the notes in the header files. I will summarize the components here.

Mega Drive
----------
megadrive.h simply hosts a generalized system init function, which is detailed within the header. It also contains `megadrive_finish()`, which terminates the sprite list and waits for vblank.

Interrupt Handlers
------------------
irq.s is where the interrupt handlers live. The vblank ISR does three things: signals the start of vblank for a synchronization function, polls and caches controller data, and processes the DMA queue with any remaining time.
The hblank and controller th pin ISRs have been left empty. If you wish to do anything with them, you will want to place a handler in irq.s, or at least call out to a C function of your choosing from here.

VDP Control
-----------
vdp.h contains VDP control functions. Most registers do not need to be manipulated directly, as separate manipulation functions exist with some level of friendliness on top. Registers written are cached, so that they may be "read back" even though the VDP does not actually support this feature. This is useful for restoring temporarily changed register values.

DMA control
-----------
dma.h contains DMA related functions. On the Mega Drive, VRAM, CRAM, and VSRAM are all written to indirectly through the VDP, as they are not memory mapped. For transferring large chunks of data, DMA is the only efficient way to do this. However, timing a DMA can be complicated, as it not only ties up the CPU bus, but faces VRAM contention during the active display region.
The DMA functions all have queued counterparts. Queued DMAs will be executed right at the start of vertical blank, and will roughly be throttled to not exceed the maximum bandwidth attainable during that time. That way, large transfers do not run into the active display area. This bandwidth "budget" is configurable.
I recommend nearly exclusively using the DMA queue instead of handling DMA timing. Of course, there will be situations where you will want to do this manually, and there are no ill effects to bypassing the DMA queue.

Palettes
--------
pal.h has functions for uploading an entire palette, or setting individual colors.

IO
--
io.h is used to get controller data. You may poll the controller directly, but the vblank ISR already does it for yuo. `io_pad_read` returns the last controller state, updated at the last vblank.

Sprites
-------
spr.h houses sprite placement functions. The sprite table is exposed under a global symbol `g_sprite_table`, but sprite placement is easy using the `spr_put` inline function. Sprites are placed into a buffer in main memory with this function. Once a frame's calculation are finished, `spr_finish` will terminate the sprite list properly, and schedule a DMA transfer of the sprite table to VRAM.

System
------
sys.h is for controlling various top level system functions. Interrupts may be disabled and enabled, as well as Z80 bus manipulation and initialization. The status register can be checked too, to retrieve the system region, revision, refresh rate, etc.

OPN and PSG
-----------
opn.h and psg.h contain functions for talking to the FM and PSG sound hardware, respectively. You may use these to create your own sound driver, but that task is typically relegated to the Z80. You might find the PSG channel pitch to be a better debugging tool than it appears at first glance.

Binary Inclusion
================

A tool called binclude is included with this project, and is a build target for the host machine. It takes a binary file and generates a .c and .h pair, with symbols named after their name and location.

If a file structure is created like this:

```
res/
    foo.bin
    bar/baz.bin
```

the following uint8_t arrays will be defined:

```
res_foo
res_bar_baz
```

They may be included with

```
#include "res_foo.h"
#include "res_bar_baz.h"
```

Hardware Testing
================

Bryan Topp's Megaloader is included, allowing for the `flash` make target to push the game binary over to a Mega Everdrive connected via USB. This is for testing your program on hardware.

TODO
====

This project is far from complete. In no particular order, there are some tasks I'd like to get taken care of:

* Z80 development
I would like for Z80 sound driver development to be a first-class endeavor. Providing a few sample drivers for different needs would not be a bad idea (include Echo, XGM, my own engine, and a simple PCM playback reference driver perhaps).
* Serial support
io.h is supposed to also allow for control of the controller ports for serial transfers. Some of the code is written, but it needs the most work and is the lowest priority.
* 6-button, mouse, and other alternate controllers
io.h should also support a variety of controllers. Right now, it operates under the assumption that a 3-button pad is used.
* DMA slicing
If a VRAM transfer is projected to overrun the remaining bandwidth, it should be split into one or more smaller transfers, deferred to the next vblank.
* Tilemap support
This is not a core function, but I would like to include a reference implementation of a simple scrolling tilemap in src/util/plane.c. Right now, plane.h only offers a function to clear planes using a DMA Fill.
