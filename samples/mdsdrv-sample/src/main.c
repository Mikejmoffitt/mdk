// mdk MDSDRV integration example.
// Michael Moffitt 2022
// This sample project illustrates integration of the MDSDRV sound driver.
// Sample music is property of the respective creators.
// https://github.com/superctr/MDSDRV

#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "mdsdrv/mdsdrv.h"
#include "res.h"

static void vbl_callback(void)
{
	mds_update();
}

void main(void)
{
	megadrive_init();

	// MDS engine initialization.
	mds_init(res_mdsseq_bin, res_mdspcm_bin);
	md_irq_register(MD_IRQ_VBLANK, vbl_callback);

	text_init(res_font_gfx_bin, sizeof(res_font_gfx_bin), 0x400, res_font_pal_bin, 0);

	// Version string and button legend.
	int text_y = 1;
	text_puts(VDP_PLANE_A, 1, text_y++, mds_get_version_str());
	text_y++;
	text_puts(VDP_PLANE_A, 1, text_y++, "C - Play");
	text_puts(VDP_PLANE_A, 1, text_y++, "B - Pause");
	text_puts(VDP_PLANE_A, 1, text_y++, "A - Stop");
	text_y++;

	MdsSlot sfx_slot = MDS_SLOT_SE1;
	uint8_t track_id = 0;
	uint8_t paused = 0;

	// Play a sample track by default.
	mds_request(MDS_SLOT_BGM, 6);

	while (1)
	{
		// Accept inputs to control the driver.
		if (g_md_pad_pos[0] & BTN_UP)
		{
			track_id--;
		}
		else if (g_md_pad_pos[0] & BTN_DOWN)
		{
			track_id++;
		}
		else if (g_md_pad_pos[0] & BTN_LEFT)
		{
			track_id -= 0x10;
		}
		else if (g_md_pad_pos[0] & BTN_RIGHT)
		{
			track_id += 0x10;
		}
		else if (g_md_pad_pos[0] & BTN_C)
		{
			if (track_id <= 6)
			{
				mds_request(MDS_SLOT_BGM, track_id);
			}
			else
			{
				mds_request(sfx_slot, track_id);
				sfx_slot++;
				if (sfx_slot > MDS_SLOT_SE3) sfx_slot = MDS_SLOT_SE1;
			}
		}
		else if (g_md_pad_pos[0] & BTN_B)
		{
			paused = !paused;
			mds_pause(MDS_SLOT_BGM, paused);
		}
		else if (g_md_pad_pos[0] & BTN_A)
		{
			mds_request(MDS_SLOT_BGM, 0);
		}

		// Print the track number.
		text_puts(VDP_PLANE_A, 1, text_y, "Track $");
		char hex_str[3];
		for (int16_t i = 0; i < 2; i++)
		{
			const uint8_t nybble = (i == 0) ?
			                       ((track_id & 0xF0) >> 4) :
			                       (track_id & 0x0F);
			if (nybble >= 0x0A) hex_str[i] = 'A' + (nybble - 0xA);
			else hex_str[i] = '0' + nybble;
		}
		hex_str[2] = '\0';
		text_puts(VDP_PLANE_A, 8, text_y, hex_str);

		// Finished with the frame.
		megadrive_finish();
	}
}
