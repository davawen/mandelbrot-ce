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

static const fixed24_t FP_ONE = 1 << FRACTION_BITS;

inline fixed24_t int_to_fp(int24_t x) {
	return (fixed24_t)x * FP_ONE;
}

inline int24_t fp_to_int(fixed24_t x) {
	return x / FP_ONE;
}

inline float fp_to_float(fixed24_t x) {
	return (float)x / (float)(1 << FRACTION_BITS);
}

inline fixed24_t naive_fp_mul(fixed24_t a, fixed24_t b) {
	return ((int32_t)a * (int32_t)b) / (1 << FRACTION_BITS);
}

inline int24_t min(int24_t a, int24_t b) {
	return (a<=b)*a + (a>b)*b;
}

inline int24_t max(int24_t a, int24_t b) {
	return (a>=b)*a + (a<b)*b;
}

inline int24_t abs(int24_t x) {
	return (x<0)*(-x) + (x>0)*x;
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

void render(uint24_t startx, uint24_t endx, uint8_t starty, uint8_t endy, fixed24_t zoom, fixed24_t offsetx, fixed24_t offsety, uint8_t step) {
	startx = (startx/step)*step; // align with step grid
	starty = (starty/step)*step;

	for (uint8_t y = starty; y < endy; y += step) {
		for (uint24_t x = startx; x < endx; x += step) {
			fixed24_t fx = fp_mul(int_to_fp(x)/HEIGHT - FP_ONE/2, zoom) + offsetx; // keep aspect ratio
			fixed24_t fy = fp_mul(int_to_fp(y)/HEIGHT - FP_ONE/2, zoom) + offsety;

			uint8_t i = mandelbrot((complex_t) { fx, fy });
			if (i == 0) {
				gfx_SetColor(0);
				gfx_FillRectangle(x, y, step, step);
			} else {
				if (i <= 7) gfx_SetColor(i);
				else if (i <= 14) gfx_SetColor(7 + i*32);
				else if (i <= 18) gfx_SetColor(231 + i*8);
				gfx_FillRectangle(x, y, step, step);
			}
		}
		gfx_BlitBuffer();
	}
}

void move_camera_x(fixed24_t *offsetx, fixed24_t amount, fixed24_t zoom, fixed24_t offsety) {
	*offsetx += fp_mul(zoom, amount);

	int24_t offset = fp_to_int(amount * HEIGHT);

	uint24_t startx = max(-offset, 0);
	uint24_t endx = min(WIDTH-offset, WIDTH);

	gfx_BlitBuffer();
	for (uint8_t y = 0; y < HEIGHT; y++) {
		for (uint24_t x = startx; x < endx; x++) {
			gfx_vbuffer[y][x] = gfx_vram[y*WIDTH + x + offset];
		}
	}

	startx = offset > 0 ? WIDTH-offset : 0;
	endx   = offset > 0 ? WIDTH        : -offset;
	render(startx, endx, 0, HEIGHT, zoom, *offsetx, offsety, 4);
}

void move_camera_y(fixed24_t *offsety, fixed24_t amount, fixed24_t zoom, fixed24_t offsetx) {
	*offsety += fp_mul(zoom, amount);

	int24_t offset = fp_to_int(amount * HEIGHT);

	uint8_t starty = max(-offset, 0);
	uint8_t endy = min((int24_t)HEIGHT-offset, HEIGHT);

	gfx_BlitBuffer();
	for (uint8_t y = starty; y < endy; y++) {
		for (uint24_t x = 0; x < WIDTH; x++) {
			gfx_vbuffer[y][x] = gfx_vram[(y+offset)*WIDTH + x];
		}
	}

	starty = offset > 0 ? HEIGHT-offset : 0;
	endy   = offset > 0 ? HEIGHT        : -offset;
	render(0, WIDTH, starty, endy, zoom, offsetx, *offsety, 4);
}

int main(void)
{
    gfx_Begin();
    gfx_SetDrawBuffer();
	gfx_FillScreen(255);

	fixed24_t zoom = FP_ONE*2;
	fixed24_t offsetx = -FP_ONE;
	fixed24_t offsety = 0;

	render(0, WIDTH, 0, HEIGHT, zoom, offsetx, offsety, 4);

	bool rendering = true;
	while (rendering) {
		kb_Scan();
		if (kb_Data[6] & kb_Clear || kb_Data[6] & kb_Annul || kb_Data[1] & kb_Del || kb_Data[1] & kb_Suppr)
			rendering = false;

		if (kb_Data[7] & kb_Left)       move_camera_x(&offsetx, -FP_ONE/4, zoom, offsety);
		else if (kb_Data[7] & kb_Right) move_camera_x(&offsetx,  FP_ONE/4, zoom, offsety); 
		else if (kb_Data[7] & kb_Up)    move_camera_y(&offsety, -FP_ONE/4, zoom, offsetx);
		else if (kb_Data[7] & kb_Down)  move_camera_y(&offsety,  FP_ONE/4, zoom, offsetx);
		else if (kb_Data[6] & kb_Enter) render(0, WIDTH, 0, HEIGHT, zoom, offsetx, offsety, 1);
		else {
			if (kb_Data[6] & kb_Add) zoom = fp_mul(zoom, FP_ONE*2/3);
			else if (kb_Data[6] & kb_Sub) zoom = fp_mul(zoom, FP_ONE*4/3);
			else continue; // don't render anything if there weren't any key press

			render(0, WIDTH, 0, HEIGHT, zoom, offsetx, offsety, 4);
		}

	}

    gfx_End();

    return 0;
}
