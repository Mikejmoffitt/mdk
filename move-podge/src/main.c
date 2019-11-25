// md-toolchain example "objay" main.c
// Michael Moffitt 2018
// Damian Yerrick 2018, 2019
//
// This main shows a character's walk cycle.

// megadrive.h is an umbrella for all headers in src/md. Specific modules like
// md/vdp.h do not need to be individually included. However, utility funcitons
// are not included, as they are not core support functions.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

extern const unsigned char obj_font_gfx_bin[];
extern const unsigned char obj_font_pal_bin[];
extern const unsigned char obj_PodgeH24_gfx_bin[];
extern const unsigned char obj_PodgeH24_pal_bin[];

/* Color notes

Per https://gendev.spritesmind.net/forum/viewtopic.php?t=2188
the VDP's DAC is somewhat nonlinear, and the actual usable levels
are 0 52 87 116 144 172 206 255.  Less shadow detail.

In web color format, these are the usable grays:
#000000, #343434, #575757, #747474, #909090, #ACACAC, #CECECE, #FFFFFF

*/

/* Player movement *************************************************/

#define WALK_SPD 105
#define WALK_ACCEL 4
#define WALK_BRAKE 8
#define LEFT_WALL 40
#define RIGHT_WALL 280

#define PLAYER_CEL_ADDR 0x8000

static unsigned int player_x;
static signed short player_dx;
static unsigned short player_frame;
static unsigned short player_facing;


static void load_player(void)
{
	player_x = 64 << 8;
	player_dx = player_frame = player_facing = 0;
	dma_q_transfer_vram(PLAYER_CEL_ADDR, (void *)obj_PodgeH24_gfx_bin, 32*6*8/2, 2);
	pal_upload(32, (void *)obj_PodgeH24_pal_bin, 16);
}

static void move_player(void)
{
	unsigned short cur_keys = io_pad_read(0);
	// Acceleration and braking while moving right
	if (player_dx >= 0)
	{
		if (cur_keys & BTN_RIGHT)
		{
			player_dx += WALK_ACCEL;
			if (player_dx > WALK_SPD) player_dx = WALK_SPD;
			player_facing = 0;	// 0: R
		}
		else
		{
			player_dx -= WALK_BRAKE;
			if (player_dx < 0) player_dx = 0;
		}
	}
	
	// Acceleration and braking while moving left
	if (player_dx <= 0)
	{
		if (cur_keys & BTN_LEFT)
		{
			player_dx -= WALK_ACCEL;
			if (player_dx < -WALK_SPD) player_dx = -WALK_SPD;
			player_facing = 1;	// 1: L
		}
		else
		{
			player_dx += WALK_BRAKE;
			if (player_dx > 0) player_dx = 0;
		}
	}

	// In a real game you'd respond to C, B, Up, Down, etc. here
	player_x += player_dx;
	
	// Test for collision with side walls
	if (player_x < (LEFT_WALL + 4) << 8)
	{
		player_x = (LEFT_WALL + 4) << 8;
		player_dx = 0;
	}
	else if (player_x >= (RIGHT_WALL - 4) << 8)
	{
		player_x = (RIGHT_WALL - 5) << 8;
		player_dx = 0;
	}
	
	// Animate the player
	if (player_dx == 0)
	{
		player_frame = 0xC0;
	}
	else
	{
		unsigned int absspeed = (player_dx < 0) ? -player_dx : player_dx;
		player_frame += absspeed * 5 / 16;

		// Wrap from end of walk cycle (7) to start of walk cycle (1)
		if (player_frame >= 0x800) player_frame -= 0x700;
	}
}

void draw_player_sprite(void)
{
	unsigned short framenum = player_frame >> 8;
	unsigned short tilenum = framenum * 6 + (PLAYER_CEL_ADDR>>5);
	unsigned short player_hotspot_x = (player_x >> 8) - 8;

	if (framenum == 7)
	{
		// This frame needs to be drawn 1 pixel forward
		player_hotspot_x += player_facing ? -1 : 1;
	}

	spr_put(player_hotspot_x, 168, SPR_ATTR(tilenum, player_facing, 0, 2, 0), SPR_SIZE(2, 3));
}

void draw_bg(void)
{
	// Set up text graphics at VRAM address 0x400 palette 0
	// This lines it up nicely with the actual ASCII values, which we will
	// use to draw letters with sprites.
	text_init(obj_font_gfx_bin, 2560, 0x400, obj_font_pal_bin, 0);

	// Print a simple message in the center of plane A
	text_puts(VDP_PLANE_A, 7, 11, "MOVE PODGE WITH THE D-PAD");

	// Draw the cubes
	for (unsigned short y = 20; y < 24; ++y)
	{
		unsigned int dest_base = VRAM_SCRA_BASE + y * 128 + 6;
		VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base);
		VDPPORT_DATA = 0x62 | (y & 1);
		VDPPORT_DATA = 0x64 | (y & 1);
		VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base + 64);
		VDPPORT_DATA = 0x62 | (y & 1);
		VDPPORT_DATA = 0x64 | (y & 1);
	}

	// Draw the floor
	unsigned int dest_base = VRAM_SCRA_BASE + 24 * 128;
	VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base);
	for (unsigned i = 64; i > 0; --i)
	{
		VDPPORT_DATA = 0x60;
	}
	for (unsigned i = 64 * 3; i > 0; --i)
	{
		VDPPORT_DATA = 0x61;
	}
}

void main(void)
{
	// Get the megadrive ready to go! (See md/megadrive.h)
	megadrive_init();

	draw_bg();
	load_player();

	while (1)
	{
		move_player();
		draw_player_sprite();

		// Set the background color to get an idea of how much CPU is utilized
		megadrive_finish(); // Terminate the sprite list and wait for vblank
		// Controller polling and DMA queue process is handled in VBL ISR
	}

}
