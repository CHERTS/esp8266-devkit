#include "cube.h"
#include "cordic.h"
#include "Adafruit_ILI9341_fast_as.h"

extern "C"
{
#include <math.h>
}

static const int16_t pix[VERTEX_COUNT][3] = {{-25,-25,-25},{25,-25,-25},{25,25,-25},{-25,25,-25},
    {-25,-25,25}, {25,-25,25}, {25,25,25}, {-25,25,25}
};

extern Adafruit_ILI9341 tft;

ICACHE_FLASH_ATTR static void calcPerspectiveProjection(const int16_t * coordinate, int16_t * x, int16_t * y)
{
    *x = coordinate[0] + coordinate[2] / 2;
    *y = coordinate[1] - coordinate[2] / 2;
}

static void drawLine( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color)
{
    tft.drawLine(x0 + 120, y0 + 160, x1 + 120, y1 + 160, color);
}

ICACHE_FLASH_ATTR void cube_calculate(int16_t _pix[VERTEX_COUNT][3], double degreeX, double degreeY, double degreeZ, double scale, int16_t shiftX, int16_t shiftY, int16_t shiftZ)
{

    uint8_t i;
    double sinx, siny, sinz, cosx, cosy, cosz;
    double x,y,z,x1,y1,z1;

    cordic(degreeX, &sinx, &cosx);
    cordic(degreeY, &siny, &cosy);
    cordic(degreeZ, &sinz, &cosz);

    for (i = 0; i < VERTEX_COUNT; i++) {

        x = pix[i][0];   // Base coordinate
        y = pix[i][1];
        z = pix[i][2];

        x1 = x*cosz + y*sinz;       // Rotation around axis Z
        y1 = -x*sinz + y*cosz;
        z1 = z;

        x = x1;                     // Rotation around axis X
        y = y1*cosx + z1*sinx;
        z = -y1*sinx + z1*cosx;

        x1 = x*cosy - z*siny;       // Rotation around axis Y
        y1 = y;
        z1 = x*siny + z*cosy;

        _pix[i][0] = (int16_t)(x1 * scale);    // Scaling
        _pix[i][1] = (int16_t)(y1 * scale);
        _pix[i][2] = (int16_t)(z1 * scale);

        _pix[i][0] += shiftX;     // Shift center coordinates
        _pix[i][1] += shiftY;
        _pix[i][2] += shiftZ;
    }
}

ICACHE_FLASH_ATTR void cube_draw(int16_t _pix[VERTEX_COUNT][3], uint16_t color)
{
    uint8_t i, j;
    int16_t x0, y0, x1, y1;

    for (j = 0; j < VERTEX_COUNT; j++) {
        i = j;
        calcPerspectiveProjection(_pix[i], &x0, &y0);
        if (i < VERTEX_COUNT/2) { // Draw sealing ribs
            calcPerspectiveProjection(_pix[i+4], &x1, &y1);
            drawLine(x0, y0, x1, y1, color);
        }
        i ++;
        i = (i == VERTEX_COUNT/2) ? 0 : i;   // Draws front face
        i = (i == VERTEX_COUNT) ? 4 : i;	// Draws back face

        calcPerspectiveProjection(_pix[i], &x1, &y1);
        drawLine(x0, y0, x1, y1, color);
    }
}

ICACHE_FLASH_ATTR void cube_draw_init()
{
}
