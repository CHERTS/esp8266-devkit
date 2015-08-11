#include "cordic.h"

static int cordic_ctab [CORDIC_NTAB] = { 0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B,
0x00FFFAAA, 0x007FFF55, 0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF,
0x0001FFFF, 0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF,
0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F, 0x0000003F, 0x0000001F, 0x0000000F,
0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000};

//#define SIN_COS_FROM_MATH_H

#ifdef SIN_COS_FROM_MATH_H
#include <math.h>
#endif

void cordic(double degree, double *s, double *c)
{

#ifdef SIN_COS_FROM_MATH_H

	*s = sin(degree / 180.0 * M_PI);
	*c = cos(degree / 180.0 * M_PI);

#else

	int k, d, tx, ty;
	int x = cordic_1K;
	int y = 0;
	int z = 0;

	if (degree < -90.0) degree += 180.0;
	if (degree > 90.0) degree -= 180.0;

	z = degree * M_PI / 180.0 * MUL;

	for (k=0; k < CORDIC_NTAB; ++k)
	{
		d = z>>31;
		//get sign. for other architectures, you might want to use the more portable version
		//d = z>=0 ? 0 : -1;
	    tx = x - (((y>>k) ^ d) - d);
	    ty = y + (((x>>k) ^ d) - d);
	    z = z - ((cordic_ctab[k] ^ d) - d);
	    x = tx; y = ty;
	}
	*c = x / MUL;
	*s = y / MUL;

#endif
}
