/*

bin2h: generate an extern definition for a bin2s symbol
for gfx/foo3.bin it'll write `extern const uint8_t gfx_foo_bin[x];` and
`extern const unsigned int gfx_foo_bin_size` to stdout, so that a header
may be collated.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv)
{
	FILE *fin;
	long int filelen;
	int linelen;
	int arg;

	if (argc < 2)
	{
		fprintf(stderr, "%s: not enough arguments", argv[0]);
		return EXIT_FAILURE;
	}

	for (arg = 1; arg < argc; arg++)
	{
		fin = fopen(argv[arg], "rb");
		if (!fin)
		{
			fprintf(stderr, "%s: could not open ", argv[0]);
			perror(argv[arg]);
			return 1;
		}

		char *filter = argv[arg];
		while (*filter != '\0')
		{
			if (*filter == '/' || *filter == '\\' ||
			    *filter == '.')
			{
				*filter = '_';
			}
			filter++;
		}

		fseek(fin, 0, SEEK_END);
		filelen = ftell(fin);
		if (filelen == 0)
		{
			fclose(fin);
			fprintf(stderr, "bin2s: warning: skipping empty file %s\n",
			        argv[arg]);
			continue;
		}
		fclose(fin);

		// The prolog for each included file has two purposes:
		// 1. align to 32-bit boundary
		// 2. provide length info (as a Pascal-like string)
		printf("extern const uint8_t %s[%d];\n", argv[arg], filelen);
		printf("extern const unsigned int %s_size;\n", argv[arg]);
	}
	return 0;
}

