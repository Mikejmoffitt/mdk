#NO_APP
	.file	"main.c"
	.text
	.section	.text.scroll_movement,"ax",@progbits
	.align	2
	.globl	scroll_movement
	.type	scroll_movement, @function
scroll_movement:
	move.l %a2,-(%sp)
	move.l %d2,-(%sp)
	clr.l -(%sp)
	lea io_pad_read,%a2
	jsr (%a2)
	addq.l #4,%sp
	and.w #8,%d0
	jeq .L2
	addq.w #1,xs
	moveq #1,%d2
.L3:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #2,%d0
	jeq .L4
	subq.w #1,ys
.L5:
	tst.w %d2
	jeq .L7
	pea 2.w
	pea 1.w
	pea xs
	move.l #63488,-(%sp)
	jsr dma_q_transfer_vram
	lea (16,%sp),%sp
.L7:
	pea 2.w
	pea 1.w
	pea ys
	clr.l -(%sp)
	jsr dma_q_transfer_vsram
	lea (16,%sp),%sp
.L1:
	move.l (%sp)+,%d2
	move.l (%sp)+,%a2
	rts
.L4:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #1,%d0
	jeq .L6
	addq.w #1,ys
	jra .L5
.L2:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	move.w %d0,%d2
	and.w #4,%d2
	jeq .L3
	subq.w #1,xs
	moveq #1,%d2
	jra .L3
.L6:
	tst.w %d2
	jeq .L1
	pea 2.w
	pea 1.w
	pea xs
	move.l #63488,-(%sp)
	jsr dma_q_transfer_vram
	lea (16,%sp),%sp
	move.l (%sp)+,%d2
	move.l (%sp)+,%a2
	rts
	.size	scroll_movement, .-scroll_movement
	.section	.text.btn_draw,"ax",@progbits
	.align	2
	.globl	btn_draw
	.type	btn_draw, @function
btn_draw:
	move.l %a2,-(%sp)
	clr.l -(%sp)
	lea io_pad_read,%a2
	jsr (%a2)
	addq.l #4,%sp
	and.w #1,%d0
	jeq .L17
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L17
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #150,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #85,4(%a0)
	move.w #144,6(%a0)
.L17:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #2,%d0
	jeq .L18
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L18
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #170,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #68,4(%a0)
	move.w #144,6(%a0)
.L18:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #4,%d0
	jeq .L19
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L19
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #160,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #76,4(%a0)
	move.w #132,6(%a0)
.L19:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #8,%d0
	jeq .L20
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L20
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #160,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #82,4(%a0)
	move.w #156,6(%a0)
.L20:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #128,%d0
	jeq .L21
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L21
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #148,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #83,4(%a0)
	move.w #204,6(%a0)
.L21:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #64,%d0
	jeq .L22
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L22
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #160,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #65,4(%a0)
	move.w #192,6(%a0)
.L22:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #16,%d0
	jeq .L23
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L23
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a1
	lsl.l #3,%d1
	move.w #160,(%a1,%d1.l)
	lea (%a1,%d1.l),%a0
	clr.b 2(%a0)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a0)
	move.w #66,4(%a0)
	move.w #204,6(%a0)
.L23:
	clr.l -(%sp)
	jsr (%a2)
	addq.l #4,%sp
	and.w #32,%d0
	jeq .L16
	move.b g_sprite_count,%d0
	cmp.b #79,%d0
	jhi .L16
	moveq #0,%d1
	move.b %d0,%d1
	lea g_sprite_table,%a2
	lsl.l #3,%d1
	move.w #160,(%a2,%d1.l)
	lea (%a2,%d1.l),%a1
	clr.b 2(%a1)
	addq.b #1,%d0
	move.b %d0,g_sprite_count
	move.b %d0,3(%a1)
	move.w #67,4(%a1)
	move.w #216,6(%a1)
.L16:
	move.l (%sp)+,%a2
	rts
	.size	btn_draw, .-btn_draw
	.section	.rodata.main.str1.1,"aMS",@progbits,1
.LC0:
	.string	"Hello World"
.LC1:
	.string	"(Scroll around with the d-pad)"
	.section	.text.startup.main,"ax",@progbits
	.align	2
	.globl	main
	.type	main, @function
main:
	movem.l #8254,-(%sp)
	jsr megadrive_init
	clr.l -(%sp)
	pea res_pal_font_bin
	pea 1024.w
	pea 3072.w
	pea res_gfx_font_bin
	jsr text_init
	lea (16,%sp),%sp
	move.l #.LC0,(%sp)
	pea 11.w
	pea 14.w
	clr.l -(%sp)
	lea text_puts,%a2
	jsr (%a2)
	lea (12,%sp),%sp
	move.l #.LC1,(%sp)
	pea 13.w
	pea 5.w
	clr.l -(%sp)
	jsr (%a2)
	lea (16,%sp),%sp
	move.l #btn_draw,%d2
	lea scroll_movement,%a6
	lea spr_finish,%a5
	lea vdp_wait_vblank,%a4
	lea io_poll,%a3
	lea dma_q_process,%a2
.L52:
	move.l %d2,%a0
	jsr (%a0)
	jsr (%a6)
	jsr (%a5)
	jsr (%a4)
	jsr (%a3)
	jsr (%a2)
	move.l %d2,%a0
	jsr (%a0)
	jsr (%a6)
	jsr (%a5)
	jsr (%a4)
	jsr (%a3)
	jsr (%a2)
	jra .L52
	.size	main, .-main
	.section	.bss.ys,"aw",@nobits
	.align	2
	.type	ys, @object
	.size	ys, 2
ys:
	.zero	2
	.section	.bss.xs,"aw",@nobits
	.align	2
	.type	xs, @object
	.size	xs, 2
xs:
	.zero	2
	.ident	"GCC: (Debian 8.3.0-2) 8.3.0"
	.section	.note.GNU-stack,"",@progbits
