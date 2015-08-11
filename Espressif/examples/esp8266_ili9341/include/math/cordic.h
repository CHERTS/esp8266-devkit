#ifndef INCLUDE_CORDIC_H_
#define INCLUDE_CORDIC_H_

//Cordic in 32 bit signed fixed point math
//Function is valid for arguments in range -pi/2 -- pi/2
//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2
//
// 1.0 = 1073741824
// 1/k = 0.6072529350088812561694

#ifdef __cplusplus
extern "C" {
#endif

//Constants
#define M_PI			3.14159265358979323846
#define cordic_1K 		0x26DD3B6A
#define half_pi 		0x6487ED51
#define MUL 			1073741824.000000
#define CORDIC_NTAB 	32

void cordic(double degree, double *s, double *c);

#ifdef __cplusplus
};
#endif

#endif /* INCLUDE_CORDIC_H_ */
