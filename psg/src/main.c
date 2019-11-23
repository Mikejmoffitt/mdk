// md-toolchain example main.c
// Michael Moffitt 2018
//
// This main shows a simple "hello world" demo.

// megadrive.h is an umbrella for all headers in src/md. Specific modules like
// md/vdp.h do not need to be individually included. However, utility funcitons
// are not included, as they are not core support functions.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

struct Chord
{
	unsigned short period[3];
	unsigned short duration;
};

// The base frequency for the PSG squarewaves is
// 39375000/(11*32) = 111860.8 Hz
// Thus the following hold:
// period_value = 111860.8/frequency
// frequency = 111860.8/period_value

static const struct Chord saygah[] =
{
	// E flat, B flat, G
	111860.8/155.5635, 111860.8/233.0819, 111860.8/391.9954, 30,
	// C, G, E
	111860.8/130.8128, 111860.8/195.9977, 111860.8/329.6276, 60,
	0, 0, 0, 0
};	

void main(void)
{
	const struct Chord *playpos = saygah;

	// Get the megadrive ready to go! (See md/megadrive.h)
	megadrive_init();

	// Set up text graphics at VRAM address 0x400 palette 0
	text_init(0x400, 0);

	// Print a simple message in the center of plane A
	text_puts(VDP_PLANE_A, 15, 11, "PSG Chords");

	// Play a sequence of chords
	for (const struct Chord *playpos = saygah;
	     playpos->duration > 0;
	     ++playpos)
	{
		// Set pitch
		for (unsigned int ch = 0; ch < 3; ++ch) {
			psg_pitch(ch, playpos->period[ch]);
		}
		
		// Fade volume
		for (unsigned int t = 0; t < playpos->duration; ++t)
		{
			for (unsigned int ch = 0; ch < 3; ++ch)
			{
				psg_vol(ch, t >> 2);
			}
			megadrive_finish();  // Wait for next frame
		}
	}

	for (unsigned int ch = 0; ch < 3; ++ch) {
		psg_vol(ch, 15);
	}

	while (1)
	{
		megadrive_finish();
	}

}
