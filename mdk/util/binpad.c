#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Pads a file to the next highest power of two.\n"
		        "Specification of a minimum size, in bytes, is optional.\n");
		printf("Usage: %s unpadded_file [min_bytes]\n", argv[0]);
		return 0;
	}

	FILE *f = fopen(argv[1], "r+");
	if (!f)
	{
		fprintf(stderr, "Couldn't open \"%s\"\n", argv[1]);
		return -1;
	}

	const size_t min_bytes = (argc >= 3) ? strtoul(argv[2], NULL, 0) : 0;

	const uint8_t pad_value = 0xFF;

	fseek(f, 0, SEEK_END);
	const size_t original_size = ftell(f);
	size_t next_size = 1;
	do
	{
		next_size = next_size * 2;
	} while (next_size < original_size);

	if (next_size < min_bytes) next_size = min_bytes;
	
	for (size_t i = 0; i < next_size - original_size; i++)
	{
		fputc(pad_value, f);
	}

	fclose(f);

	return 0;
}
