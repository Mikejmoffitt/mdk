#include "types.h"

#include <stdio.h>
#include <string.h>

static ConvOrigin conv_origin_from_string(const char *argstr)
{
	ConvOrigin ret = CONV_ORIGIN_DEFAULT;
	switch (argstr[0])
	{
		case 'l':
		case 'L':
			ret = CONV_ORIGIN_LEFT_TOP;
			break;
		case 'c':
		case 'C':
			ret = CONV_ORIGIN_CENTER_TOP;
			break;
		case 'r':
		case 'R':
			ret = CONV_ORIGIN_RIGHT_TOP;
			break;
		default:
			printf("Warning: Unhandled X origin argument '%c'.\n", argstr[0]);
			return ret;
	}

	switch (argstr[1])
	{
		case 't':
		case 'T':
			switch (ret)
			{
				case CONV_ORIGIN_LEFT_TOP:
					ret = CONV_ORIGIN_LEFT_TOP;
					break;
				case CONV_ORIGIN_CENTER_TOP:
					ret = CONV_ORIGIN_CENTER_TOP;
					break;
				case CONV_ORIGIN_RIGHT_TOP:
					ret = CONV_ORIGIN_RIGHT_TOP;
					break;
				case CONV_ORIGIN_LEFT_CENTER:
					ret = CONV_ORIGIN_LEFT_TOP;
					break;
				case CONV_ORIGIN_CENTER_CENTER:
					ret = CONV_ORIGIN_CENTER_TOP;
					break;
				case CONV_ORIGIN_RIGHT_CENTER:
					ret = CONV_ORIGIN_RIGHT_TOP;
					break;
				case CONV_ORIGIN_LEFT_BOTTOM:
					ret = CONV_ORIGIN_LEFT_TOP;
					break;
				case CONV_ORIGIN_CENTER_BOTTOM:
					ret = CONV_ORIGIN_CENTER_TOP;
					break;
				case CONV_ORIGIN_RIGHT_BOTTOM:
					ret = CONV_ORIGIN_RIGHT_TOP;
					break;
			}
			break;
		case 'c':
		case 'C':
			switch (ret)
			{
				case CONV_ORIGIN_LEFT_TOP:
					ret = CONV_ORIGIN_LEFT_CENTER;
					break;
				case CONV_ORIGIN_CENTER_TOP:
					ret = CONV_ORIGIN_CENTER_CENTER;
					break;
				case CONV_ORIGIN_RIGHT_TOP:
					ret = CONV_ORIGIN_RIGHT_CENTER;
					break;
				case CONV_ORIGIN_LEFT_CENTER:
					ret = CONV_ORIGIN_LEFT_CENTER;
					break;
				case CONV_ORIGIN_CENTER_CENTER:
					ret = CONV_ORIGIN_CENTER_CENTER;
					break;
				case CONV_ORIGIN_RIGHT_CENTER:
					ret = CONV_ORIGIN_RIGHT_CENTER;
					break;
				case CONV_ORIGIN_LEFT_BOTTOM:
					ret = CONV_ORIGIN_LEFT_CENTER;
					break;
				case CONV_ORIGIN_CENTER_BOTTOM:
					ret = CONV_ORIGIN_CENTER_CENTER;
					break;
				case CONV_ORIGIN_RIGHT_BOTTOM:
					ret = CONV_ORIGIN_RIGHT_CENTER;
					break;
			}
			break;
		case 'b':
		case 'B':
			switch (ret)
			{
				case CONV_ORIGIN_LEFT_TOP:
					ret = CONV_ORIGIN_LEFT_BOTTOM;
					break;
				case CONV_ORIGIN_CENTER_TOP:
					ret = CONV_ORIGIN_CENTER_BOTTOM;
					break;
				case CONV_ORIGIN_RIGHT_TOP:
					ret = CONV_ORIGIN_RIGHT_BOTTOM;
					break;
				case CONV_ORIGIN_LEFT_CENTER:
					ret = CONV_ORIGIN_LEFT_BOTTOM;
					break;
				case CONV_ORIGIN_CENTER_CENTER:
					ret = CONV_ORIGIN_CENTER_BOTTOM;
					break;
				case CONV_ORIGIN_RIGHT_CENTER:
					ret = CONV_ORIGIN_RIGHT_BOTTOM;
					break;
				case CONV_ORIGIN_LEFT_BOTTOM:
					ret = CONV_ORIGIN_LEFT_BOTTOM;
					break;
				case CONV_ORIGIN_CENTER_BOTTOM:
					ret = CONV_ORIGIN_CENTER_BOTTOM;
					break;
				case CONV_ORIGIN_RIGHT_BOTTOM:
					ret = CONV_ORIGIN_RIGHT_BOTTOM;
					break;
			}
			break;
		default:
			printf("Warning: Unhandled Y origin argument '%c'.\n", argstr[1]);
			return ret;
	}
	return ret;
}

ConvOrigin conv_origin_from_args(int argc, char **argv)
{
	if (argc < 7) return CONV_ORIGIN_DEFAULT;
	const char *argstr = argv[6];
	if (strlen(argstr) < 2)
	{
		printf("Warning: Invalid origin '%s'; need two characters.\n", argstr);
		return CONV_ORIGIN_DEFAULT;
	}

	return conv_origin_from_string(argstr);
}
