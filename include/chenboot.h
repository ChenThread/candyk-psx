/*
chenboot: crt0 and interrupt bootstrap facilities for the PlayStation
Copyright (C) GreaseMonkey, 2017, licensed under Creative Commons Zero:
https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <stdint.h>

//
// Interrupt Service Routine handling
//
typedef struct chenboot_exception_frame {
	uint32_t epc;
	uint32_t thread_ra;
	uint32_t thread_at;
	uint32_t thread_v[2];
	uint32_t thread_a[4];
	uint32_t thread_t[10];
	uint32_t sr;
	uint32_t cause;
} __attribute__((__packed__)) chenboot_exception_frame_t;

extern chenboot_exception_frame_t *(*chenboot_isr_hook)(chenboot_exception_frame_t *sp);
chenboot_exception_frame_t *chenboot_isr_default(chenboot_exception_frame_t *sp);
void chenboot_isr_install(
	chenboot_exception_frame_t *(*hook_cb)(
		chenboot_exception_frame_t *sp));
void chenboot_isr_enable(void);
void chenboot_isr_disable(void);

