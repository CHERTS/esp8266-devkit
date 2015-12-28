#ifndef INCLUDE_CUBE_H_
#define INCLUDE_CUBE_H_

#define VERTEX_COUNT 	8

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <osapi.h>

void cube_calculate(int16_t _pix[VERTEX_COUNT][3], double degreeX, double degreeY, double degreeZ, double scale, int16_t shiftX, int16_t shiftY, int16_t shiftZ);
void cube_draw(int16_t _pix[VERTEX_COUNT][3], uint16_t color);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_CUBE_H_ */
