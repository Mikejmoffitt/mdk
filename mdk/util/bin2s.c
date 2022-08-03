/*

bin2s: convert a binary file to an ARM asm module
for gfx/foo3.bin it'll write gfx_foo_bin (an array of char)
and gfx_foo_bin_len (an unsigned int)
for 4bit.chr it'll write _4bit_chr and _4bit_chr_len


Copyright 2003, 2019 Damian Yerrick

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/



/*
.align
.global SomeLabel_len
.int 1234
.global SomeLabel
.byte blah,blah,blah,blah...
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* strnident() *************************
   Print the closest valid C identifier to a given word.
*/
void strnident(FILE *fout, const char *src)
{
  char got_first = 0;

  while (*src != 0) {
    int s = *src++;

    // Add an underscore before an initial digit
    if (isdigit(s) && !got_first) {
      fputc('_', fout);  /* stick a '_' before an initial digit */
    }

    /* convert out-of-range characters */
    if (!isalpha(s) && !isdigit(s)) {
      s = '_';
    }

    if (s) {
      fputc(s, fout);
      got_first = 1;
    }
  }
}

const char usageText[] =
  "usage: bin2s foo.bin bar.bin baz.bin > foo.s\n"
  "Converts binary files to GNU assembly language.\n"
  "Each object is named as the file path with non-alphanumeric\n"
  "characters converted to underscores.  For example, 'res/kitten.chr'\n"
  "becomes 'res_kitten_chr'.  Then each file is written as two symbols:\n"
  " 1. The symbol with '_size' appended (e.g. 'res_kitten_chr_size')\n"
  "    Points to a uint32_t holding the file's length in bytes\n"
  " 2. The symbol itself (e.g. 'res_kitten_chr')\n"
  "    Points to file's contents\n"
  "The sizes are 32-bit aligned, and the contents always directly follow\n"
  "the size, so a program can treat them as Pascal strings and traverse\n"
  "a set of files linearly.\n"
;

const char versionText[] =
  "bin2s 19.11\n"
  "Copyright 2003, 2019 Damian Yerrick.  Comes with NO WARRANTY.\n"
  "This is free software under the MIT/Expat license.\n"
;

// Metadata procured from an optional external metadata file.
typedef struct FileMeta {
  int alignment;
} FileMeta;

// Initializes metadata from (optional) filename with derivative path.
void initialize_meta(const char *base_fname, FileMeta *meta) {
  memset(meta, 0, sizeof(*meta));
  meta->alignment = 2;  // Default.

  char meta_fname[256];
  snprintf(meta_fname, sizeof(meta_fname), "%s.cfg", base_fname);
  FILE *f = fopen(meta_fname, "r");
  if (!f) return;

  fseek(f, 0, SEEK_END);
  const size_t flen = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *fbuf = malloc(flen);
  if (!fbuf) {
    fprintf(stderr, "cfg file of size $%X couldn't be loaded\n");
    fclose(f);
    return;
  }

  fread(fbuf, 1, flen, f);
  fclose(f);

  static const char *delim = " \t\n\r";

  char *fbuf_orig = fbuf;

  strtok(fbuf, delim);
  do {
    if (strcmp(fbuf, "align") == 0) {
      fbuf = strtok(NULL, delim);
      if (!fbuf) break;
      meta->alignment = strtoul(fbuf, 0, 0);
    }
  }
  while (fbuf = strtok(NULL, delim));

  free(fbuf_orig);
}

int main(int argc, char **argv) {
  FILE *fin;
  long int filelen;
  int linelen;
  int arg;

  if (argc < 2) {
    fputs("bin2s: not enough arguments; try bin2s --help\n", stderr);
    return EXIT_FAILURE;
  }

  FileMeta meta;

  if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")
      || !strcmp(argv[1], "-?")) {
    fputs(usageText, stdout);
    return 0;
  }

  for (arg = 1; arg < argc; arg++) {
    // Omit optional ".cfg" files
    if (strstr(argv[arg], ".cfg")) continue;
    initialize_meta(argv[arg], &meta);
    fin = fopen(argv[arg], "rb");
    if (!fin) {
      fputs("bin2s: could not open ", stderr);
      perror(argv[arg]);
      return 1;
    }

    fseek(fin, 0, SEEK_END);
    filelen = ftell(fin);
    if (filelen == 0) {
      fclose(fin);
      fprintf(stderr, "bin2s: warning: skipping empty file %s\n",
              argv[arg]);
      continue;
    }
    rewind(fin);

    // The prolog for each included file has two purposes:
    // 1. align to 32-bit boundary
    // 2. provide length info (as a Pascal-like string)
    fprintf(stdout,
          "/* Generated by BIN2S - please don't edit directly */\n"
          ".section .rodata\n"
          ".balign %d\n"
          ".global ", meta.alignment);
    strnident(stdout, argv[arg]);
    fputs("_size\n", stdout);
    strnident(stdout, argv[arg]);
    printf("_size: .dc.l %lu\n.global ", (unsigned long)filelen);
    strnident(stdout, argv[arg]);
    fputs("\n", stdout);
    strnident(stdout, argv[arg]);
    fputs(":\n.byte ", stdout);

    linelen = 0;
    while (filelen > 0) {
      unsigned char c = fgetc(fin);

      printf("%3u", (unsigned int)c);
      filelen--;

      /* don't put a comma after the last item */
      if (filelen > 0) {
        /* break after every 16th number */
        if (++linelen >= 16) {
          linelen = 0;
          fputs("\n.byte ", stdout);
        } else {
          fputc(',', stdout);
        }
      }
    }
    fputc('\n', stdout);

    fclose(fin);
  }
  return 0;
}

