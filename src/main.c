////////////////////////////////////////
// { BAD APPLE } { 0.1.0 }
// Author: davawen
// License: MIT
// Description: d
////////////////////////////////////////

// for autocomplete
#ifndef __INT24_TYPE__
#define __INT24_TYPE__
#define __UINT24_TYPE__
#endif

#include <debug.h>

#include <stdint.h>
#include <time.h>

#include <graphx.h>
#include <keypadc.h>

#include "fastmul.h"

const uint24_t WIDTH = GFX_LCD_WIDTH;
const uint8_t HEIGHT = GFX_LCD_HEIGHT;

typedef int24_t fixed;
#define FRACTION_BITS 12

inline fixed int_to_fp(uint24_t x) {
	return (fixed)x << FRACTION_BITS;
}

float fp_to_float(fixed x) {
	return (float)x / (float)(1 << FRACTION_BITS);
}

fixed naive_fp_mul(fixed a, fixed b) {
	return ((int32_t)a * (int32_t)b) / (1 << FRACTION_BITS);
}

typedef struct {
	fixed real;
	fixed imag;
} complex;

// (a+ib)^2
// a^2 + 2aib - b^2

// a^2 - b^2
// (a+b)(a-b)

/// returns how many interations
uint8_t mandelbrot(complex c) {
	complex z = { 0, 0 };

	dbg_printf("c = %li + i%li = %f + i%f\n", c.real, c.imag, fixed_to_float(c.real), fixed_to_float(c.imag));
	for (uint8_t i = 0; i < 10; i++) {
		dbg_printf("z = %li + i%li = %f + i%f\n", z.real, z.imag, fixed_to_float(z.real), fixed_to_float(z.imag));
		if (z.real >= int_to_fp(2) || z.imag >= int_to_fp(2)) return i;

		z = (complex) {
			fp_mul(z.real + z.imag, z.real - z.imag),
			2*fp_mul(z.real, z.imag)
		};

		z.real += c.real;
		z.imag += c.imag;
	}

	return 0;
}

int main(void)
{
    gfx_Begin();
    gfx_SetDrawBuffer();
	gfx_FillScreen(255);

	int24_t zoom = 1;
	for (uint8_t y = 0; y < HEIGHT; y+=4) {
		for (uint24_t x = 0; x < WIDTH; x+=4) {
			fixed fx = int_to_fp(x)*3/WIDTH  / zoom - int_to_fp(2);
			fixed fy = int_to_fp(y)*2/HEIGHT / zoom - int_to_fp(1);

			uint8_t i = mandelbrot((complex) { fx, fy });
			if (i == 0) {
				dbg_printf("got the mandel-bro!\n");
				gfx_SetColor(0);
				gfx_FillRectangle(x, y, 4, 4);
			} else if (i < 8) {
				gfx_SetColor(7 + i*32);
				gfx_FillRectangle(x, y, 4, 4);
			}
		}
		gfx_BlitBuffer();
		// kb_Scan();
		// if (kb_Data[6] & kb_Enter) {
		// 	zoom += 1;
		// }
	}

	while (!kb_AnyKey()) {}

	gfx_SetColor(0);
    gfx_End();

    return 0;
}
