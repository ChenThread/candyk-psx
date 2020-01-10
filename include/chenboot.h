/*
chenboot: crt0 and interrupt bootstrap facilities for the PlayStation

Copyright (c) 2017 Ben "GreaseMonkey" Russell

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
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

