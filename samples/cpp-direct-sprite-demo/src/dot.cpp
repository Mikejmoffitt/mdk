#include "dot.h"

#include "md/megadrive.h"

#include "util/rand.h"

namespace
{
	static constexpr fix16_t kGravity = INTTOFIX16(0.08);
	static constexpr fix32_t kBoundLeft = INTTOFIX32(4);
	static constexpr fix32_t kBoundRight = INTTOFIX32(320 - 4);
	static constexpr fix32_t kGroundY = INTTOFIX32(224 - 7);
	static constexpr fix16_t kMinimumDy = kGravity * 16;
	static constexpr fix16_t kBounceDyCoef = INTTOFIX16(0.75);

	static int s_next_slot;
}  // namespace

Dot::Dot()
{
	// The Dot objects use a static int to assign themselves sprite slots.
	if (s_next_slot >= MD_SPR_MAX) s_next_slot = 0;
	m_spr = &g_sprite_table[s_next_slot];
	s_next_slot++;

	// The sprite attributes that will not change are assigned values once.
	m_spr->link = s_next_slot;
	m_spr->attr = SPR_ATTR(1, 0, 0, 0, 0);
	m_spr->size = SPR_SIZE(1, 1);

	ResetVelocity();
}

void Dot::ResetVelocity()
{
	m_dx = INTTOFIX16((rand() % 64) - 32) / 16;
	m_dy = INTTOFIX16((rand() % 32) - 48) / 8;
}

void Dot::Move()
{
	m_x += m_dx;
	m_y += m_dy;
	m_dy += kGravity;

	if (m_y > kGroundY && m_dy > 0)
	{
		m_y = kGroundY;
		if (m_dy < kMinimumDy) ResetVelocity();
		else m_dy = FIX16MUL(m_dy, -kBounceDyCoef);
	}

	if (m_x > kBoundRight && m_dx > 0)
	{
		m_dx = -m_dx;
		m_x = kBoundRight;
	}
	else if (m_x < kBoundLeft && m_dx < 0)
	{
		m_dx = -m_dx;
		m_x = kBoundLeft;
	}
}

void Dot::Render()
{
	m_spr->xpos = FIX16TOINT(m_x) + 128;
	m_spr->ypos = FIX16TOINT(m_y) + 128;
}
