#ifndef TYPES_H
#define TYPES_H

#define PCG_TILE_PX 16

// Enum for the origin point of an image.
typedef enum ConvOrigin
{
	CONV_ORIGIN_LEFT_TOP,
	CONV_ORIGIN_CENTER_TOP,
	CONV_ORIGIN_RIGHT_TOP,
	CONV_ORIGIN_LEFT_CENTER,
	CONV_ORIGIN_CENTER_CENTER,
	CONV_ORIGIN_RIGHT_CENTER,
	CONV_ORIGIN_LEFT_BOTTOM,
	CONV_ORIGIN_CENTER_BOTTOM,
	CONV_ORIGIN_RIGHT_BOTTOM,
	CONV_ORIGIN_DEFAULT = CONV_ORIGIN_CENTER_CENTER
} ConvOrigin;

// Enum for the conversion mode.
typedef enum ConvMode
{
	CONV_MODE_AUTO,
	CONV_MODE_XOBJ,
	CONV_MODE_SP,
	CONV_MODE_DEFAULT = CONV_MODE_AUTO
} ConvMode;

ConvOrigin conv_origin_from_args(int argc, char **argv);

#endif  // TYPES_H
