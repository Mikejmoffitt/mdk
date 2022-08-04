// md-toolchain example "audio-simple-psg" main.c
// Damian Yerrick 2019
// Michael Moffitt 2022
//
// This demo plays two chords through the programmable sound
// generator (PSG) that the MD inherits from the Master System.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "res.h"

struct Chord
{
	unsigned short period[3];
	unsigned short duration;
};

// Notes about tuning:
// NTSC defines the color burst frequency as 35/44 of the 4.5 MHz
// audio carrier offset, that is, 315/88 MHz or about 3.58 MHz.
// The PSG divides this by 32 to form the basic frequency for its
// three square waves.
// 315/88 MHz / 32 = 111860.8 Hz
// Thus the following hold:
// period_value = 111860.8/frequency
// frequency = 111860.8/period_value

static const struct Chord s_sega_chords[] =
{
	// E flat, B flat, G, half a second
	{{(111860.8/155.5635), (111860.8/233.0819), (111860.8/391.9954)}, 50},
	// C, G, E, one second
	{{(111860.8/130.8128), (111860.8/195.9977), (111860.8/329.6276)}, 60},
	{{0, 0, 0}, 0}
};

void main(void)
{
	megadrive_init();
	text_init(res_font_gfx_bin, 3072, 0x400, res_font_pal_bin, 0);
	text_puts(VDP_PLANE_A, 15, 11, "PSG Chords");

	// Play a sequence of chords.
	for (const struct Chord *playpos = s_sega_chords;
	     playpos->duration > 0;
	     playpos++)
	{
		// Set pitch
		for (unsigned int ch = 0; ch < 3; ch++)
		{
			md_psg_pitch(ch, playpos->period[ch]);
		}
		
		// Fade volume
		for (unsigned int t = 0; t < playpos->duration; t++)
		{
			for (unsigned int ch = 0; ch < 3; ch++)
			{
				md_psg_vol(ch, t >> 2);
			}
			megadrive_finish();  // Wait for next frame
		}
	}

	// Silence all channels with max attenuation (15).
	for (unsigned int ch = 0; ch < 3; ch++) md_psg_vol(ch, 15);

	while (1)
	{
		megadrive_finish();
	}
}
