/*
chenboot: crt0 and interrupt bootstrap facilities for the PlayStation
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/

TODO: interrupt bootstrap facilities
*/
	.section .text
	.set noreorder

	.extern main
	.extern _gp

	.global _start
_start:
	/* Preset some regs */
	la $gp, _gp

	/* Jump to main */
	jal main
	nop

	/* INFINITE LOOP */
	1:
	j 1b
	nop

