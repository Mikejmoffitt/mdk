MDK: Mega Drive Software Development Kit
=======================================

Origin
------
This project was born out of my wanting to write my own C support code for Mega Drive when I was working on a particular game.
I had developed it in another SDK, but between a mismatch in code style, old code not aging well, and some functionally completely breaking when I updated to a newer version, I wanted to feel a little more in control of my project.
So, I started fresh, and tried to compartmentalize all of my code that interacted with the MD hardware, keeping it divorced from the game logic where sensible.
This project continues to receive updates as I add features or change it during the development of that game.

IT IS NOT YET STABLE! The API may change at any time, until that game is done.


Goals
-----
This project aims to supply a set of minimal support code for use in making software for Sega Mega Drive, Genesis, System C, and System C2 platforms using the C programming language.
Current solutions exist on both ends of the complexity spectrum, with straight 68000 assembly programming on one and SGDK on the other.
This project aims to deliver something a little more in the middle in terms of weight and simplicity.

This project supports Linux, Windows, and MacOS-based development. For Windows, I strongly suggest using Windows Subsytem for Linux.

Rather than provide a library to link against, this project is intended to act as a skeleton that is the basis of another project.
MD support functions may be compiled and used. The suggested use case is to make an `mdk` submodule for a project.

Sample code and Makefiles exist in the `samples/` directory. 
A makefile is provided that will recursively look for source files in the `src/` directory. It links using the provided linker script, which puts ROM from $000000 to $3FFFFF and RAM at $FF0000-$FFFFFF.

Setup
=====

Environment Prerequisites
-------------------------
The toolchain is provided by git@github.com:andwn/marsdev.
There are two options to set it up:

Option 1: Use my install script
-------------------------------
I put together a script to set up the M68000 GCC toolchain from marsdev. It will clone and build GCC on your machine.

As a prerequisite, you will need a host build environment appropriate for building GCC.

I use Debian, so the following packages are appropriate for me:

```

    $ sudo apt install build-essential git texinfo

```

Run install-toolchain.sh. This script
1) Clones marsdev into a directory in /tmp
2) Creates /opt/mdk-toolchain
3) Builds GCC, binutils, and friends with marsdev
4) Moves m68k-elf into /opt-mdk-toolchain/
5) Cleans up after itself in /tmp

Option 2: Build marsdev yourself
--------------------------------
Clone git@github.com:andwn/marsdev:

```
    $ git clone git@github.com:andwn/marsdev
```

Build the toolchain:

```
    $ cd marsdev
    $ make m68k-toolchain-newlib LANGS=c,c++
```

Move the necessary files to the toolchain directory:

```
    $ sudo mkdir -p /opt/mdk-toolchain
    $ mv ~/marsdev/m68k-elf /opt/mdk-toolchain/
    $ rm -rf ~/marsdev
```

If you don't want to use /opt/mdk-toolchain (e.g. you wish to just use ~/marsdev, or something else) you can set the environment variable MDK_BASE to point at the root containing m68k-elf/.

Starting a Project
------------------

If you are starting fresh, I recommend copying the contents of blank-project into your root and using that:

```
    $ cp -r mdk/blank-project my-project
```

Then, add MDK as a submodule:

```
    $ cd my-project
    $ git submodule add git@github.com:mikejmoffitt/mdk mdk
    $ git add mdk && git commit -m "Added MDK submodule."
```

The default Makefile searches in the `src` directory recursively for any .c files or .s files, and compiles or assembles them accordingly.
In addition, files placed in `res` are included in binary form. Their usage is detailed further below.
Edit the Makefile to change the name from blank-project to that if your project.

At this stage, you should be able to build your project, though it will not do anything interesting.

```
    $ make
```

It's not required to do so, but you may want to edit mdk/header.inc to include your project metadata as well.

You can periodically update MDK with the following command:

```
    $ git submodule update --remote --merge
    $ git add mdk && git commit -m "Updated MDK submodule."
```

Extra Dependencies
------------------

If you have edited your Makefile to include additional generated files (script parsing, data generation, etc), you should declare any external build dependencies with `EXTERNAL_DEPS`, and make the build system aware of externally generated artifacts with `EXTERNAL_ARTIFACTS` (so they can be cleaned when the `clean` target is run).

Utils
-----

MDK relies on a few host-side utilities, so you will want to include `utils/` in your project.

SDK Usage
=========

Paths and Sources
-----------------
MDK presents two directories in your source path: `md`, which contains the core Mega Drive-related functionality, and `util`, which has useful auxiliary functions that aren't considered core functions.

Different components are in `md`, and their header files will provide useful structures, enums, and documentation.
However, `md/megadrive.h` exists to declare a function which initializes all of these components, and if included you may avoid referencing each component separately.

Interaction
-----------
The C runtime startup code clears RAM, sets up initial data for static variables, sets up the stack, and jumps to `main()`.
That's all that is done out of the C programmer's sight. However, I'd recommend making the very first line in `main()` a call to `megadrive_init()`:

```
    megadrive_init();
```

This sets the Mega Drive to sane defaults, and initializes various helper systems for the peripherals.
Following that, an infinite loop like this is sufficient:

```
    while(1)
    {
        // Your logic goes here!
        megadrive_finish();
    }
```

Samples
-------

Check out the samples/ directory for some small example projects that show usage of the SDK.

Platforms and Unused Code
-------------------------

MDK presently supports the following platforms:
* Mega Drive / Sega Genesis
* Sega System C2

The default platform is Mega Drive, and the symbol MDK_TARGET_MD is defined.
You may specify another platform by setting the following in the Makefile:

```
    TARGET_PLATFORM := MDK_TARGET_C2
```

Multiple platforms means that some SDK code may not be relevant to your platform.
If the code is not relevant, it is not called, or the branch to determine whether to call it is extremly cheap (i.e. a single `tst.w`)
As for code size, GCC does a good job deleting unused functions and variables, so code size will not suffer.
As for memory usage, unnecessary variables are gated by #ifdef guards, and will not waste RAM.

Components
==========
For each component, I recommend reading the notes in the header
files. I will summarize the components here.

Platform
--------

`megadrive_finish()` should be called at the end of one frame of execution\*, and will do the following things:
  * Terminate the sprite list
  * Queue any CRAM (palette) memory transfers
  * Wait for vertical blank to begin
  * Poll controllers and store button state
  * Process pending DMA transfers (to VRAM, CRAM, or VSRAM)

You can see the body of `megadrive_finish()` in md/megadrive.h. As with almost everything in this project, you aren't required to use it. Feel free to copy the parts you like, or call them elsewhere, if you really want to.

\* Small note on execution timing and vblank: On a system with fixed specifications like the Mega Drive, it is not only normal but also recommended that one "tick" of game engine execution occur synchronously with the refresh of the screen.
As a result, it's typical that one frame of logic runs, and then waits for the end of the frame. I am not doing anything from stopping you from running your game based on timer reads from the OPN or something, if you really want to.

Interrupt Handlers
------------------
`irq.s` is where the interrupt handlers live.  The vblank ISR does
three things: signals the start of vblank for a synchronization
function, polls and caches controller data, and processes the DMA
queue with any remaining time.

The hblank and controller th pin ISRs have been left empty. If you
wish to do anything with them, you will want to place a handler in
`irq.s`, or at least call out to a C function of your choosing from
here.

VDP Control
-----------
`vdp.h` contains VDP control functions.  Most registers do not need
to be manipulated directly, as separate manipulation functions exist
with some level of friendliness on top.  Registers written are
cached, so that they may be "read back" even though the VDP does not
actually support this feature.  This is useful for restoring
temporarily changed register values.

DMA control
-----------
`dma.h` contains DMA related functions.  On the Mega Drive, VRAM,
CRAM, and VSRAM are all written to indirectly through the VDP, as
they are not memory mapped. For transferring large chunks of data,
DMA is the only efficient way to do this. However, timing a DMA can
be complicated, as it not only ties up the CPU bus, but faces VRAM
contention during the active display region.

DMA transfers are registered with calls to the DMA queue functions, and that list is processed during vertical blank (from `md_dma_process()`).
A single high-priority slot exists for the sake of the sprite table, which will be checked and transferred before any others.

If you really want a DMA to occur immediately, you may call `md_dma_process()` right after queueing a transfer.

Palettes
--------
`pal.h` has functions for uploading an entire palette, or setting individual colors.
It operates on a 64-entry palette cache in work RAM, and keeps track of whether a palette "line" (16-color grouping native to the video system) has changed.
Accordingly, it will queue a transfer with DMA to be done during vertical blank.

IO
--
`io.h` is used to get controller data. It supports 3 and 6 button controllers, and SMS controllers with a caveat.

A six-button pad is detected with a simple herusitic, and a special bit is set in the returned data (MD_PAD_6B).
You may test this bit to determine if a six-button pad is in use.

If no pad is plugged in, `MD_PAD_UNPLUGGED` is set in the controller data.
If an SMS controller is in use, this bit may get set as a side effect.
It is safe to ignore this bit.

Sprites
-------
`spr.h` houses sprite placement functions. The sprite table is
exposed under a global symbol `g_sprite_table`, but sprite placement
is easy using the `md_spr_put()` inline function.  Sprites are placed into
a buffer in main memory with this function.  Once a frame's
calculation is finished, `md_spr_finish()` will terminate the sprite list
properly, and schedule a DMA transfer of the sprite table to VRAM.

`md_spr_mask_line_full` will insert a "magic sprite" that masks off any other sprites appearing on the specified scanline.

`md_spr_finish()` is called by `megadrive_finish()`, so if you are using that, you do not need to worry about it.

System
------
`sys.h` is for controlling various top level system functions.
Interrupts may be disabled and enabled, as well as Z80 bus
manipulation and initialization. The status register can be checked
too, to retrieve the system region, revision, refresh rate, etc.

Interrupts and Exceptions
-------------------------
`irq.h` presents `md_irq_register()` by which a callback can be associated with interrupts and excepptions.


```
    void irq_callback(void);
    { ... }

    md_irq_register(MD_IRQ_VBLANK, irq_callback);
```

They are called safely from an interrupt context, with register clobber protection.

If you are a geek and wrote an interrupt handler in assembly, and know how to safely treat registers, you may call `md_irq_register_unsafe()` instead.

The vertical blank ISR clears a flag related to frame timing, and then calls the callback if it is there. Registering a callback will not interfere with normal operation.

Sound
-----------
`opn.h` and `psg.h` contain functions for talking to the FM and PSG
sound hardware, respectively.  You may use these to create your own
sound driver, but that task is typically relegated to the Z80.  You
might find the PSG channel pitch to be a better debugging tool than
it appears at first glance.

Including Binary Data
=====================

A tool called `bin2s` is included with this project, and is a build
target for the host machine.  It takes several binary files and
generates an assembly language (.s) file containing their data, with
symbols named after their name and location.

If a file structure is created like this:

```
res/
    foo.bin
    bar/baz.bin
```

the following `uint8_t` arrays will be defined:

```
extern const uint8_t res_foo_bin[];
extern const uint8_t res_bar_baz_bin[];
```

The following sizes will also be defined:

```
extern const uint32_t res_foo_bin_size;
extern const uint32_t res_bar_baz_bin_size;
```

If you wish to generate files using other tools that will then be placed in `res/`, you can declare them as external dependencies.

Hardware Testing
================

Bryan Topp's Megaloader is included, allowing for the `flash` make
target to push the game binary over to a Mega Everdrive connected via
USB. This is for testing your program on hardware.

TODO
====

This project is far from complete. In no particular order, there are
some tasks I'd like to get taken care of:

* **Z80 development**
  I would like for Z80 sound driver development to be a first-class
  endeavor. Providing a few sample drivers for different needs would
  not be a bad idea (include Echo, XGM, my own engine, and a simple
  PCM playback reference driver perhaps).
* **DMA slicing**
  If a VRAM transfer is projected to overrun the remaining bandwidth,
  it should be split into one or more smaller transfers, deferred to
  the next vblank.
* **Tilemap functions**
  This is not a core function, but I would like to include a
  reference implementation of a simple scrolling tilemap in
  `src/util/plane.c`.  Right now, `plane.h` only offers a function
  to clear planes using a DMA Fill.
* **Bank switching support**
  Allow placing object files in specific sections that get mapped
  to specific 512 KiB banks for use with a Sega mapper or 2 MiB
  banks for use with a simpler discrete mapper.
