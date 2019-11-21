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

#define WALK_SPD 105
#define WALK_ACCEL 4
#define WALK_BRAKE 8
#define LEFT_WALL 40
#define RIGHT_WALL 280

static unsigned int player_x;
static signed short player_dx;
static unsigned short player_frame;
static unsigned short player_facing = 0;

static void load_player(void) {
//  LZ77UnCompVram(spritegfx_chrTiles, SPR_VRAM(16));
  player_x = 64 << 8;
  player_dx = player_frame = player_facing = 0;
}

static void move_player(void) {
  unsigned short cur_keys = io_pad_read(0);
  // Acceleration and braking while moving right
  if (player_dx >= 0) {
    if (cur_keys & BTN_RIGHT) {
      player_dx += WALK_ACCEL;
      if (player_dx > WALK_SPD) player_dx = WALK_SPD;
      player_facing = 0;  // 0: R
    } else {
      player_dx -= WALK_BRAKE;
      if (player_dx < 0) player_dx = 0;
    }
  }
  
  // Acceleration and braking while moving left
  if (player_dx <= 0) {
    if (cur_keys & BTN_LEFT) {
      player_dx -= WALK_ACCEL;
      if (player_dx < -WALK_SPD) player_dx = -WALK_SPD;
      player_facing = 1;  // 1: L
    } else {
      player_dx += WALK_BRAKE;
      if (player_dx > 0) player_dx = 0;
    }
  }

  // In a real game you'd respond to C, B, Up, Down, etc. here
  player_x += player_dx;
  
  // Test for collision with side walls
  if (player_x < (LEFT_WALL + 4) << 8) {
    player_x = (LEFT_WALL + 4) << 8;
    player_dx = 0;
  } else if (player_x >= (RIGHT_WALL - 4) << 8) {
    player_x = (RIGHT_WALL - 5) << 8;
    player_dx = 0;
  }
  
  // Animate the player
  if (player_dx == 0) {
    player_frame = 0xC0;
  } else {
    unsigned int absspeed = (player_dx < 0) ? -player_dx : player_dx;
    player_frame += absspeed * 5 / 16;

    // Wrap from end of walk cycle (7) to start of walk cycle (1)
    if (player_frame >= 0x800) player_frame -= 0x700;
  }
}


void draw_player_sprite(void)
{
	unsigned short framenum = player_frame >> 8;
	unsigned short tilenum = framenum * 6 + 0x40;
	unsigned short player_hotspot_x = (player_x >> 8) - 8;

	if (framenum == 7)
	{
		// Frame 1 needs to be drawn 1 pixel forward
		player_hotspot_x += player_facing ? -1 : 1;
	}

	spr_put(player_hotspot_x, 160, SPR_ATTR(tilenum, player_facing, 0, 0, 0), SPR_SIZE(2, 3));
}

void main(void)
{
	// Get the megadrive ready to go! (See md/megadrive.h)
	megadrive_init();

	// Set up text graphics at VRAM address 0x400 palette 0
	// This lines it up nicely with the actual ASCII values, which we will
	// use to draw letters with sprites.
	text_init(0x400, 0);

	// Print a simple message in the center of plane A
	text_puts(VDP_PLANE_A, 14, 11, "Hiiii");
	for (unsigned short y = 19; y < 23; ++y)
	{
		text_puts(VDP_PLANE_A, 3, y, "[]");
		text_puts(VDP_PLANE_A, 35, y, "[]");
	}
	text_puts(VDP_PLANE_A, 0, 23, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

	while (1)
	{
		move_player();
		draw_player_sprite();

		// Set the background color to get an idea of how much CPU is utilized
		megadrive_finish(); // Terminate the sprite list and wait for vblank
		// Controller polling and DMA queue process is handled in VBL ISR
	}

}
