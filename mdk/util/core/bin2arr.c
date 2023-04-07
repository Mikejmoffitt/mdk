#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const int MAX_ITEMS_PER_LINE = 16;

int main(int argc, char **argv)
{
	int max_items_per_line = MAX_ITEMS_PER_LINE;
	size_t bytes_written = 0;
	int out_count_per_line = 0;
	char out_fpath[512];
	char *symbol_name;
	FILE *fin, *fout;
	if (argc < 3)
	{
		fprintf(stderr, "Usage:\n%s <binary file> <symbol name> "
		                "[items per line]\n", argv[0]);
		return -1;
	}

	if (argc > 3)
	{
		max_items_per_line = atoi(argv[3]);
		if (max_items_per_line < 1)
		{
			fprintf(stderr, "Warning: Invalid value \"%s\" provided for \
			                 max items per line. Defaulting to %d.\n",
			                 argv[3], MAX_ITEMS_PER_LINE);
			max_items_per_line = MAX_ITEMS_PER_LINE;
		}
	}

	symbol_name = argv[2];

	fin = fopen(argv[1], "rb");
	if (!fin)
	{
		fprintf(stderr, "Couldn't open %s for reading.\n", argv[1]);
		return -1;
	}

	snprintf(out_fpath, sizeof(out_fpath) - 1, "%s.c", symbol_name);
	out_fpath[sizeof(out_fpath) - 1] = 0;
	fout = fopen(out_fpath, "w");
	if (!fout)
	{
		fprintf(stderr, "Couldn't open %s for writing.\n", out_fpath);
		fclose(fin);
		return -1;
	}

	// Write const array to C file
	fseek(fin, 0L, SEEK_END);
	const size_t binsize = ftell(fin);
	fseek(fin, 0L, SEEK_SET);
	fprintf(fout, "#include <stdint.h>\nconst uint8_t %s[%d] =\n{",
	        symbol_name, (int)binsize);

	while (!feof(fin))
	{
		if (out_count_per_line == 0)
		{
			fprintf(fout, "\n");
			out_count_per_line = max_items_per_line;
		}

		out_count_per_line--;

		int fetch = fgetc(fin);
		if (fetch == EOF)
		{
			break;
		}
		if (out_count_per_line >= max_items_per_line - 1)
		{
			fprintf(fout, "\t");
		}
		fprintf(fout, "0x%02X,", fetch);
		bytes_written++;
	}
	fprintf(fout, "\n};\n\n");
	fclose(fout);
	fclose(fin);

	return 0;
}
