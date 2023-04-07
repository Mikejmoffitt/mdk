#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void split(const char *in_fname, const char *out_even_fname, const char *out_odd_fname, int bytes)
{
	int c;
	FILE *fin, *fout_odd, *fout_even;
	fin = fopen(in_fname, "rb");
	if (!fin)
	{
		fprintf(stderr, "Couldn't open %s.\n", in_fname);
		return;
	}
	fout_odd = fopen(out_odd_fname, "wb");
	if (!fout_odd)
	{
		fprintf(stderr, "Couldn't open %s.\n", out_odd_fname);
		fclose(fin);
		return;
	}
	fout_even = fopen(out_even_fname, "wb");
	if (!fout_even)
	{
		fprintf(stderr, "Couldn't open %s.\n", out_even_fname);
		fclose(fin);
		fclose(fout_odd);
		return;
	}

	do
	{
		for (int i = 0; i < bytes; i++)
		{
			c = fgetc(fin);
			if (feof(fin))
			{
				goto done;
			}
			fputc(c, fout_even);
		}
		for (int i = 0; i < bytes; i++)
		{
			c = fgetc(fin);
			if (feof(fin))
			{
				goto done;
			}
			fputc(c, fout_odd);
		}
	}
	while (c != EOF);
done:
	fclose(fin);
	fclose(fout_odd);
	fclose(fout_even);
}

void combine(const char *in_even_fname, const char *in_odd_fname, const char *out_fname, int bytes)
{
	int c;
	FILE *fin_odd, *fin_even, *fout;
	fout = fopen(out_fname, "wb");
	if (!fout)
	{
		fprintf(stderr, "Couldn't open %s.\n", out_fname);
		return;
	}
	fin_even = fopen(in_even_fname, "rb");
	if (!fin_even)
	{
		fprintf(stderr, "Couldn't open %s.\n", in_even_fname);
		fclose(fout);
		return;
	}
	fin_odd = fopen(in_odd_fname, "rb");
	if (!fin_odd)
	{
		fprintf(stderr, "Couldn't open %s.\n", in_odd_fname);
		fclose(fin_even);
		fclose(fout);
		return;
	}

	do
	{
		for (int i = 0; i < bytes; i++)
		{
			c = fgetc(fin_even);
			if (c != EOF)
			{
				fputc(c, fout);
			}
		}

		for (int i = 0; i < bytes; i++)
		{
			c = fgetc(fin_odd);
			if (c != EOF)
			{
				fputc(c, fout);
			}
		}
	} while (c != EOF);

	fclose(fout);
	fclose(fin_even);
	fclose(fin_odd);
}

void exchange(const char *in_fname, const char *out_fname)
{
	int c;
	uint8_t byte[2];
	FILE *fin, *fout;
	fout = fopen(out_fname, "wb");
	if (!fout)
	{
		fprintf(stderr, "Couldn't open %s.\n", out_fname);
		return;
	}
	fin = fopen(in_fname, "rb");
	if (!fin)
	{
		fprintf(stderr, "Couldn't open %s.\n", in_fname);
		fclose(fout);
		return;
	}

	do
	{
		c = fgetc(fin);
		if (c == EOF)
		{
			break;
		}
		byte[0] = c;
		c = fgetc(fin);
		if (c == EOF)
		{
			break;
		}
		byte[1] = c;

		fputc(byte[1], fout);
		fputc(byte[0], fout);
	} while (c != EOF);

	fclose(fout);
	fclose(fin);
}

void exchange_nybbles(const char *in_fname, const char *out_fname)
{
	int c;
	FILE *fin, *fout;
	fout = fopen(out_fname, "wb");
	if (!fout)
	{
		fprintf(stderr, "Couldn't open %s.\n", out_fname);
		return;
	}
	fin = fopen(in_fname, "rb");
	if (!fin)
	{
		fprintf(stderr, "Couldn't open %s.\n", in_fname);
		fclose(fout);
		return;
	}

	do
	{
		c = fgetc(fin);
		if (c == EOF)
		{
			break;
		}
		fputc(((c & 0xF) << 4) | ((c & 0xF0) >> 4), fout);
	} while (c != EOF);

	fclose(fout);
	fclose(fin);
}

void exchange_halfnybbles(const char *in_fname, const char *out_fname)
{
	int c;
	uint8_t byte;
	FILE *fin, *fout;
	fout = fopen(out_fname, "wb");
	if (!fout)
	{
		fprintf(stderr, "Couldn't open %s.\n", out_fname);
		return;
	}
	fin = fopen(in_fname, "rb");
	if (!fin)
	{
		fprintf(stderr, "Couldn't open %s.\n", in_fname);
		fclose(fout);
		return;
	}

	do
	{
		c = fgetc(fin);
		if (c == EOF)
		{
			break;
		}
		byte = c;
		c = ((byte & 0x03) << 2) | ((byte & 0x0C) >> 2) | (((byte & 0x30) << 2) | ((byte & 0xC0) >> 2));

		fputc(c, fout);
	} while (c != EOF);

	fclose(fout);
	fclose(fin);
}

void usage(const char *name)
{
	printf("Usage: %s [op]\n", name);
	printf("    split: s in out.even out.odd <bytes per>\n");
	printf("  combine: c in.even in.odd out <bytes per>\n");
	printf(" exchange: x in out\n");
	printf(" ex. 4bit: n in out\n");
	printf(" ex. 2bit: z in out\n");
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 0;
	}

	int bytes = 1;

	if (argc > 5)
	{
		bytes = atoi(argv[5]);
		printf("Using %d bytes interleave cadence\n", bytes);
	}

	if (argv[1][0] == 's')
	{
		if (argc < 5)
		{
			usage(argv[0]);
			return 0;
		}
		printf("Splitting\n");
		split(argv[2], argv[3], argv[4], bytes);
	}
	else if (argv[1][0] == 'c')
	{
		if (argc < 5)
		{
			usage(argv[0]);
			return 0;
		}
		printf("Combining\n");
		combine(argv[2], argv[3], argv[4], bytes);
	}
	else if (argv[1][0] == 'x')
	{
		if (argc < 4)
		{
			usage(argv[0]);
			return 0;
		}
		printf("Exchanging bytes\n");
		exchange(argv[2], argv[3]);
	}
	else if (argv[1][0] == 'n')
	{
		if (argc < 4)
		{
			usage(argv[0]);
			return 0;
		}
		printf("Exchanging nybbles\n");
		exchange_nybbles(argv[2], argv[3]);
	}
	else if (argv[1][0] == 'z')
	{
		if (argc < 4)
		{
			usage(argv[0]);
			return 0;
		}
		printf("Exchanging half-nybbles\n");
		exchange_halfnybbles(argv[2], argv[3]);
	}

	return 0;
}
