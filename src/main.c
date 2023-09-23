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
#include <stdbool.h>

#include <graphx.h>
#include <keypadc.h>

#include "fastmul.h"

const uint24_t WIDTH = GFX_LCD_WIDTH;
const uint8_t HEIGHT = GFX_LCD_HEIGHT;

typedef int24_t fixed24_t;
#define FRACTION_BITS 12

inline fixed24_t int_to_fp(uint24_t x) {
	return (fixed24_t)x << FRACTION_BITS;
}

static const fixed24_t FP_ONE = 1 << FRACTION_BITS;

float fp_to_float(fixed24_t x) {
	return (float)x / (float)(1 << FRACTION_BITS);
}

fixed24_t naive_fp_mul(fixed24_t a, fixed24_t b) {
	return ((int32_t)a * (int32_t)b) / (1 << FRACTION_BITS);
}

typedef struct {
	fixed24_t real;
	fixed24_t imag;
} complex_t;

#define MAX_ITERATIONS 18

/// returns how many interations it took to diverge or 0 if c is in the set
uint8_t mandelbrot(complex_t c) {
	static const fixed24_t fp_two = FP_ONE*2;

	complex_t z = { 0, 0 };

	for (uint8_t i = 0; i < MAX_ITERATIONS; i++) {
		if (z.real >= fp_two || z.imag >= fp_two) return i;

		// (a+ib)^2
		// a^2 - b^2 + 2aib 

		// a^2 - b^2
		// (a+b)(a-b)
		z = (complex_t) {
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

	int24_t zoom = FP_ONE*2;
	int24_t offsetx = -FP_ONE;
	int24_t offsety = 0;
	bool rendering = true;
	while (rendering) {
		for (uint8_t y = 0; y < HEIGHT; y += 4) {
			for (uint24_t x = 0; x < WIDTH; x += 4) {
				fixed24_t fx = fp_mul(int_to_fp(x)/HEIGHT - FP_ONE/2, zoom) + offsetx; // keep aspect ratio
				fixed24_t fy = fp_mul(int_to_fp(y)/HEIGHT - FP_ONE/2, zoom) + offsety;

				uint8_t i = mandelbrot((complex_t) { fx, fy });
				if (i == 0) {
					gfx_SetColor(0);
					gfx_FillRectangle(x, y, 4, 4);
				} else {
					if (i <= 7) gfx_SetColor(i);
					else if (i <= 14) gfx_SetColor(7 + i*32);
					else if (i <= 18) gfx_SetColor(231 + i*8);
					gfx_FillRectangle(x, y, 4, 4);
				}
			}
			gfx_BlitBuffer();
		}

		while (true) {
			kb_Scan();
			if (kb_Data[6] & kb_Clear || kb_Data[6] & kb_Annul || kb_Data[1] & kb_Del || kb_Data[1] & kb_Suppr)
				rendering = false;
			else if (kb_Data[6] & kb_Add) zoom = fp_mul(zoom, FP_ONE*2/3);
			else if (kb_Data[6] & kb_Sub) zoom = fp_mul(zoom, FP_ONE*4/3);
			else if (kb_Data[7] & kb_Left)  offsetx -= fp_mul(zoom, FP_ONE/4);
			else if (kb_Data[7] & kb_Right) offsetx += fp_mul(zoom, FP_ONE/4);
			else if (kb_Data[7] & kb_Up)    offsety -= fp_mul(zoom, FP_ONE/4);
			else if (kb_Data[7] & kb_Down)  offsety += fp_mul(zoom, FP_ONE/4);
			else continue;

			break;
		}
	}

    gfx_End();

    return 0;
}
