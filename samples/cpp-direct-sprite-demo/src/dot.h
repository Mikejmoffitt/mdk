#ifndef DOT_H
#define DOT_H

#include "util/fixed.h"
#include "md/megadrive.h"

class Dot
{
public:
	Dot();
	void Move();
	void Render();
private:
	void ResetVelocity();

	fix32_t m_x = INTTOFIX32(160);
	fix32_t m_y = INTTOFIX32(200);
	fix16_t m_dx = 0;
	fix16_t m_dy = 0;

	SprSlot *m_spr;  // Initialized in Dot().

};

#endif  // DOT_H
