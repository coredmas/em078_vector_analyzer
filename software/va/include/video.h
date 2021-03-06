/*
 * video.h
 *
 *  Created on: 26 ���. 2018 �.
 *      Author: user
 */

#ifndef VIDEO_H_
#define VIDEO_H_

#include <stdint.h>
#include "../include/ff.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define WIDTH 1024
#define HEIGHT 768

void setcolor(uint8_t r, uint8_t g, uint8_t b);
void drawpixel(int32_t x, int32_t y);
void drawline(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
double hue2rgb(double p, double q, double tt);
uint8_t hslToR(double h, double s, double l);
uint8_t hslToG(double h, double s, double l);
uint8_t hslToB(double h, double s, double l);
void drawtext(const char * string, int size, uint16_t x, uint16_t y);
void clrscr(void);
void videoinit(void);
void swapbuffers(void);

#endif /* VIDEO_H_ */
