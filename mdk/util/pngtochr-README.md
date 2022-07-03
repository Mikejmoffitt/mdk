pngtochr
========

This is a program to convert indexed PNG images to the character
(CHR) data that 8- and 16-bit video game consoles use.  It describes
each data format using a string called a "plane map," which allows
one program to cover multiple consoles.

This is a rewrite of pilbmp2nes.py, one of the tools included in
my NES and Super NES project templates, in the C language.  It has
some advantages over the Python version:

- Faster on large images
- Does not need the Python 3 interpreter installed
- Smaller, especially on platforms where Python is uncommon

Disadvantages:

- Cannot read non-PNG images
- Cannot be imported as a library
- Not tested as much

Legal
-----
Copyright 2019 Damian Yerrick

The main program and the LodePNG library are under the zlib License
as set forth in `lodepng.cpp`.  The option parser from the musl library
is under the MIT (Expat) License as set forth in `musl_getopt.c`.
