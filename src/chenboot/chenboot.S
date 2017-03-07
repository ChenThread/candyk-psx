/*
chenboot: crt0 and interrupt bootstrap facilities for the PlayStation
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/

#define I_STAT 0x1070
#define I_MASK 0x1074

#define FRAME_EPC   ( 0*4)
#define FRAME_SR    (19*4)
#define FRAME_CAUSE (20*4)

	.text
	.set noreorder

	.extern main
	.extern _gp
	.extern _edata
	.extern _end

	.global _start
	.global chenboot_isr_default
	.global chenboot_isr_install
	.global chenboot_isr_disable
	.global chenboot_isr_enable

	.ent __chenboot_isr
	.type __chenboot_isr, @function
__chenboot_isr:
	/* Prologue */
	.set noat
	mfc0  $k1, $14
	addiu $sp, $sp, -32*4
	sw    $k1, 0*4($sp)
	sw    $ra, 1*4($sp)
	sw    $at, 2*4($sp)
	sw    $v0, 3*4($sp)
	sw    $v1, 4*4($sp)
	sw    $a0, 5*4($sp)
	sw    $a1, 6*4($sp)
	sw    $a2, 7*4($sp)
	sw    $a3, 8*4($sp)
	sw    $t0, 9*4($sp)
	sw    $t1,10*4($sp)
	sw    $t2,11*4($sp)
	sw    $t3,12*4($sp)
	sw    $t4,13*4($sp)
	sw    $t5,14*4($sp)
	sw    $t6,15*4($sp)
	sw    $t7,16*4($sp)
	sw    $t8,17*4($sp)
	sw    $t9,18*4($sp)

	/* Load sr/cause */
	mfc0  $t0, $12
	nop
	mfc0  $t1, $13
	nop
	sw    $t0,FRAME_SR($sp)
	sw    $t1,FRAME_CAUSE($sp)

	/* Jump */
	la    $t2, __chenboot_isr_hook
	lw    $t3, 0($t2)
	move  $a0, $sp
	jalr  $t3
	addiu $sp, $sp, -32
	addiu $sp, $sp,  32

	/* Epilogue */
	lw    $v0, 3*4($sp)
	lw    $v1, 4*4($sp)
	lw    $a0, 5*4($sp)
	lw    $a1, 6*4($sp)
	lw    $a2, 7*4($sp)
	lw    $a3, 8*4($sp)
	lw    $t0, 9*4($sp)
	lw    $t1,10*4($sp)
	lw    $t2,11*4($sp)
	lw    $t3,12*4($sp)
	lw    $t4,13*4($sp)
	lw    $t5,14*4($sp)
	lw    $t6,15*4($sp)
	lw    $t7,16*4($sp)
	lw    $t8,17*4($sp)
	lw    $t9,18*4($sp)

	lw    $at, 2*4($sp)
	lw    $ra, 1*4($sp)
	lw    $k0, 0*4($sp)
	addiu $sp, $sp, 32*4
	jr    $k0
	rfe
	.set at
	.end __chenboot_isr

	.ent __chenboot_isr_template
	.type __chenboot_isr_template, @function
__chenboot_isr_template:
	j __chenboot_isr
	.end __chenboot_isr_template

	.ent chenboot_isr_install
	.type chenboot_isr_install, @function
chenboot_isr_install:
	/* Prologue */
	addiu $sp, $sp, -16
	sw    $ra, 0x00($sp)
	
	/* Change ISR hook */
	la $t0, __chenboot_isr_hook
	sw $a0, 0($t0)

	/* Replace interrupt handler */
	la $t0, __chenboot_isr_template
	lw $t2, 0($t0)
	lui $t1, 0x8000
	sw $t2, 0x0080($t1)

	/* Call FlushCache */
	addiu $sp, $sp, -16
	la    $t0, 0xA0
	jalr  $t0
	li    $t1, 0x44
	addiu $sp, $sp, 16

	/* Epilogue */
	lw    $ra, 0x00($sp)
	nop
	jr    $ra
	addiu $sp, $sp, 16
	.end chenboot_isr_install

	.ent chenboot_isr_default
	.type chenboot_isr_default, @function
chenboot_isr_default:
	/* Read cause register */
	/* Lock up if not an interrupt */
	lw    $t0, FRAME_CAUSE($a0)
	nop
	andi  $t0, $t0, 0x00FC
	1:
		bne $t0, $zero, 1b
		nop

	/* Clear all interrupts */
	lui   $t0, 0x1F80
	lh    $t1, I_STAT($t0)
	nop
	nor   $t1, $t1, $t1
	sh    $t1, I_STAT($t0)

	/* Return */
	jr    $ra
	nop
	.end chenboot_isr_default

	.ent chenboot_isr_disable
	.type chenboot_isr_disable, @function
chenboot_isr_disable:
	/* Clear Im2+IEc */
	mfc0  $t0, $12
	li    $t1, ~0x0401
	and   $t0, $t0, $t1
	mtc0  $t0, $12

	/* Return */
	jr    $ra
	nop
	.end chenboot_isr_disable

	.ent chenboot_isr_enable
	.type chenboot_isr_enable, @function
chenboot_isr_enable:
	/* Set Im2+IEc */
	mfc0  $t0, $12
	li    $t1, 0x0401
	or    $t0, $t0, $t1
	mtc0  $t0, $12

	/* Return */
	jr    $ra
	nop
	.end chenboot_isr_enable

	.ent _start
	.type _start, @function
_start:
	/* Preset some regs */
	la $gp, _gp

	/* Clear heap */
	la    $t0, _edata
	la    $t1, _end
	subu  $t1, $t1, $t0    /* t1 to delta */
	addiu $t1, $t1, 0x000F /* 16-byte align, round up */
	ori   $t1, $t1, 0x000F
	xori  $t1, $t1, 0x000F
	addu  $t1, $t1, $t0    /* t1 to ptr */
	1:
		sw    $zero, 0x00($t0)
		sw    $zero, 0x04($t0)
		sw    $zero, 0x08($t0)
		sw    $zero, 0x0C($t0)
		bne   $t0,   $t1, 1b
		addiu $t0,   $t0, 0x10

	/* Jump to main */
	jal main
	nop

	/* INFINITE LOOP */
	1:
	j 1b
	nop
	.end _start

	.data
	.ent __chenboot_isr_hook
__chenboot_isr_hook:
	.word chenboot_isr_default
	.end __chenboot_isr_hook

