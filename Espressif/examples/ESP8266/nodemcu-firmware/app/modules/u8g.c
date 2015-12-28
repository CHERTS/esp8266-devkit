// Module for U8glib

#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

//#include "c_string.h"
#include "c_stdlib.h"

#include "u8g.h"

#include "u8g_config.h"

struct _lu8g_userdata_t
{
    u8g_t     u8g;
    u8g_pb_t  pb;
    u8g_dev_t dev;
};

typedef struct _lu8g_userdata_t lu8g_userdata_t;

// shorthand macro for the u8g structure inside the userdata
#define LU8G (&(lud->u8g))


// helper function: retrieve and check userdata argument
static lu8g_userdata_t *get_lud( lua_State *L )
{
    lu8g_userdata_t *lud = (lu8g_userdata_t *)luaL_checkudata(L, 1, "u8g.display");
    luaL_argcheck(L, lud, 1, "u8g.display expected");
    return lud;
}

// helper function: retrieve given number of integer arguments
static void lu8g_get_int_args( lua_State *L, uint8_t stack, uint8_t num, u8g_uint_t *args)
{
    while (num-- > 0)
    {
        *args++ = luaL_checkinteger( L, stack++ );
    }
}


// Lua: u8g.begin( self )
static int lu8g_begin( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_Begin( LU8G );

    return 0;
}

// Lua: u8g.setFont( self, font )
static int lu8g_setFont( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_fntpgm_uint8_t *font = (u8g_fntpgm_uint8_t *)lua_touserdata( L, 2 );
    if (font != NULL)
        u8g_SetFont( LU8G, font );

    return 0;
}

// Lua: u8g.setFontRefHeightAll( self )
static int lu8g_setFontRefHeightAll( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontRefHeightAll( LU8G );

    return 0;
}

// Lua: u8g.setFontRefHeightExtendedText( self )
static int lu8g_setFontRefHeightExtendedText( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontRefHeightExtendedText( LU8G );

    return 0;
}

// Lua: u8g.setFontRefHeightText( self )
static int lu8g_setFontRefHeightText( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontRefHeightText( LU8G );

    return 0;
}

// Lua: u8g.setDefaultBackgroundColor( self )
static int lu8g_setDefaultBackgroundColor( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetDefaultBackgroundColor( LU8G );

    return 0;
}

// Lua: u8g.setDefaultForegroundColor( self )
static int lu8g_setDefaultForegroundColor( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetDefaultForegroundColor( LU8G );

    return 0;
}

// Lua: u8g.setFontPosBaseline( self )
static int lu8g_setFontPosBaseline( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontPosBaseline( LU8G );

    return 0;
}

// Lua: u8g.setFontPosBottom( self )
static int lu8g_setFontPosBottom( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontPosBottom( LU8G );

    return 0;
}

// Lua: u8g.setFontPosCenter( self )
static int lu8g_setFontPosCenter( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontPosCenter( LU8G );

    return 0;
}

// Lua: u8g.setFontPosTop( self )
static int lu8g_setFontPosTop( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetFontPosTop( LU8G );

    return 0;
}

// Lua: int = u8g.getFontAscent( self )
static int lu8g_getFontAscent( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetFontAscent( LU8G ) );

    return 1;
}

// Lua: int = u8g.getFontDescent( self )
static int lu8g_getFontDescent( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetFontDescent( LU8G ) );

    return 1;
}

// Lua: int = u8g.getFontLineSpacing( self )
static int lu8g_getFontLineSpacing( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetFontLineSpacing( LU8G ) );

    return 1;
}

// Lua: int = u8g.getMode( self )
static int lu8g_getMode( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetMode( LU8G ) );

    return 1;
}

// Lua: u8g.setColorIndex( self, color )
static int lu8g_setColorIndex( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetColorIndex( LU8G, luaL_checkinteger( L, 2 ) );

    return 0;
}

// Lua: int = u8g.getColorIndex( self )
static int lu8g_getColorIndex( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetColorIndex( LU8G ) );

    return 1;
}

static int lu8g_generic_drawStr( lua_State *L, uint8_t rot )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[2];
    lu8g_get_int_args( L, 2, 2, args );

    const char *s = luaL_checkstring( L, (1+2) + 1 );
    if (s == NULL)
        return 0;

    switch (rot)
    {
    case 1:
        lua_pushinteger( L, u8g_DrawStr90( LU8G, args[0], args[1], s ) );
        break;
    case 2:
        lua_pushinteger( L, u8g_DrawStr180( LU8G, args[0], args[1], s ) );
        break;
    case 3:
        lua_pushinteger( L, u8g_DrawStr270( LU8G, args[0], args[1], s ) );
        break;
    default:
        lua_pushinteger( L, u8g_DrawStr( LU8G, args[0], args[1], s ) );
        break;
    }

    return 1;
}


// Lua: pix_len = u8g.drawStr( self, x, y, string )
static int lu8g_drawStr( lua_State *L )
{
    lu8g_userdata_t *lud;

    return lu8g_generic_drawStr( L, 0 );
}

// Lua: pix_len = u8g.drawStr90( self, x, y, string )
static int lu8g_drawStr90( lua_State *L )
{
    lu8g_userdata_t *lud;

    return lu8g_generic_drawStr( L, 1 );
}

// Lua: pix_len = u8g.drawStr180( self, x, y, string )
static int lu8g_drawStr180( lua_State *L )
{
    lu8g_userdata_t *lud;

    return lu8g_generic_drawStr( L, 2 );
}

// Lua: pix_len = u8g.drawStr270( self, x, y, string )
static int lu8g_drawStr270( lua_State *L )
{
    lu8g_userdata_t *lud;

    return lu8g_generic_drawStr( L, 3 );
}

// Lua: u8g.drawLine( self, x1, y1, x2, y2 )
static int lu8g_drawLine( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    u8g_DrawLine( LU8G, args[0], args[1], args[2], args[3] );

    return 0;
}

// Lua: u8g.drawTriangle( self, x0, y0, x1, y1, x2, y2 )
static int lu8g_drawTriangle( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[6];
    lu8g_get_int_args( L, 2, 6, args );

    u8g_DrawTriangle( LU8G, args[0], args[1], args[2], args[3], args[4], args[5] );

    return 0;
}

// Lua: u8g.drawBox( self, x, y, width, height )
static int lu8g_drawBox( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    u8g_DrawBox( LU8G, args[0], args[1], args[2], args[3] );

    return 0;
}

// Lua: u8g.drawRBox( self, x, y, width, height, radius )
static int lu8g_drawRBox( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[5];
    lu8g_get_int_args( L, 2, 5, args );

    u8g_DrawRBox( LU8G, args[0], args[1], args[2], args[3], args[4] );

    return 0;
}

// Lua: u8g.drawFrame( self, x, y, width, height )
static int lu8g_drawFrame( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    u8g_DrawFrame( LU8G, args[0], args[1], args[2], args[3] );

    return 0;
}

// Lua: u8g.drawRFrame( self, x, y, width, height, radius )
static int lu8g_drawRFrame( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[5];
    lu8g_get_int_args( L, 2, 5, args );

    u8g_DrawRFrame( LU8G, args[0], args[1], args[2], args[3], args[4] );

    return 0;
}

// Lua: u8g.drawDisc( self, x0, y0, rad, opt = U8G_DRAW_ALL )
static int lu8g_drawDisc( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[3];
    lu8g_get_int_args( L, 2, 3, args );

    u8g_uint_t opt = luaL_optinteger( L, (1+3) + 1, U8G_DRAW_ALL );

    u8g_DrawDisc( LU8G, args[0], args[1], args[2], opt );

    return 0;
}

// Lua: u8g.drawCircle( self, x0, y0, rad, opt = U8G_DRAW_ALL )
static int lu8g_drawCircle( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[3];
    lu8g_get_int_args( L, 2, 3, args );

    u8g_uint_t opt = luaL_optinteger( L, (1+3) + 1, U8G_DRAW_ALL );

    u8g_DrawCircle( LU8G, args[0], args[1], args[2], opt );

    return 0;
}

// Lua: u8g.drawEllipse( self, x0, y0, rx, ry, opt = U8G_DRAW_ALL )
static int lu8g_drawEllipse( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    u8g_uint_t opt = luaL_optinteger( L, (1+4) + 1, U8G_DRAW_ALL );

    u8g_DrawEllipse( LU8G, args[0], args[1], args[2], args[3], opt );

    return 0;
}

// Lua: u8g.drawFilledEllipse( self, x0, y0, rx, ry, opt = U8G_DRAW_ALL )
static int lu8g_drawFilledEllipse( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    u8g_uint_t opt = luaL_optinteger( L, (1+4) + 1, U8G_DRAW_ALL );

    u8g_DrawFilledEllipse( LU8G, args[0], args[1], args[2], args[3], opt );

    return 0;
}

// Lua: u8g.drawPixel( self, x, y )
static int lu8g_drawPixel( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[2];
    lu8g_get_int_args( L, 2, 2, args );

    u8g_DrawPixel( LU8G, args[0], args[1] );

    return 0;
}

// Lua: u8g.drawHLine( self, x, y, width )
static int lu8g_drawHLine( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[3];
    lu8g_get_int_args( L, 2, 3, args );

    u8g_DrawHLine( LU8G, args[0], args[1], args[2] );

    return 0;
}

// Lua: u8g.drawVLine( self, x, y, width )
static int lu8g_drawVLine( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[3];
    lu8g_get_int_args( L, 2, 3, args );

    u8g_DrawVLine( LU8G, args[0], args[1], args[2] );

    return 0;
}

// Lua: u8g.drawXBM( self, x, y, width, height, data )
static int lu8g_drawXBM( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    const char *xbm_data = luaL_checkstring( L, (1+4) + 1 );
    if (xbm_data == NULL)
        return 0;

    u8g_DrawXBM( LU8G, args[0], args[1], args[2], args[3], (const uint8_t *)xbm_data );

    return 0;
}

// Lua: u8g.drawBitmap( self, x, y, count, height, data )
static int lu8g_drawBitmap( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t args[4];
    lu8g_get_int_args( L, 2, 4, args );

    const char *bm_data = luaL_checkstring( L, (1+4) + 1 );
    if (bm_data == NULL)
        return 0;

    u8g_DrawBitmap( LU8G, args[0], args[1], args[2], args[3], (const uint8_t *)bm_data );

    return 0;
}

// Lua: u8g.setScale2x2( self )
static int lu8g_setScale2x2( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetScale2x2( LU8G );

    return 0;
}

// Lua: u8g.undoScale( self )
static int lu8g_undoScale( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_UndoScale( LU8G );

    return 0;
}

// Lua: u8g.firstPage( self )
static int lu8g_firstPage( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_FirstPage( LU8G );

    return 0;
}

// Lua: bool = u8g.nextPage( self )
static int lu8g_nextPage( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushboolean( L, u8g_NextPage( LU8G ) );

    return 1;
}

// Lua: u8g.sleepOn( self )
static int lu8g_sleepOn( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SleepOn( LU8G );

    return 0;
}

// Lua: u8g.sleepOff( self )
static int lu8g_sleepOff( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SleepOff( LU8G );

    return 0;
}

// Lua: u8g.setRot90( self )
static int lu8g_setRot90( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetRot90( LU8G );

    return 0;
}

// Lua: u8g.setRot180( self )
static int lu8g_setRot180( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetRot180( LU8G );

    return 0;
}

// Lua: u8g.setRot270( self )
static int lu8g_setRot270( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_SetRot270( LU8G );

    return 0;
}

// Lua: u8g.undoRotation( self )
static int lu8g_undoRotation( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_UndoRotation( LU8G );

    return 0;
}

// Lua: width = u8g.getWidth( self )
static int lu8g_getWidth( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetWidth( LU8G ) );

    return 1;
}

// Lua: height = u8g.getHeight( self )
static int lu8g_getHeight( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetHeight( LU8G ) );

    return 1;
}

// Lua: width = u8g.getStrWidth( self, string )
static int lu8g_getStrWidth( lua_State *L )
{
   lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    const char *s = luaL_checkstring( L, 2 );
    if (s == NULL)
        return 0;

    lua_pushinteger( L, u8g_GetStrWidth( LU8G, s ) );

    return 1;
}

// Lua: u8g.setFontLineSpacingFactor( self, factor )
static int lu8g_setFontLineSpacingFactor( lua_State *L )
{
   lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    u8g_uint_t factor = luaL_checkinteger( L, 2 );

    u8g_SetFontLineSpacingFactor( LU8G, factor );

    return 0;
}


// ------------------------------------------------------------
// comm functions
//
#define I2C_CMD_MODE    0x000
#define I2C_DATA_MODE   0x040

#define ESP_I2C_ID 0


static uint8_t do_i2c_start(uint8_t id, uint8_t sla)
{
    platform_i2c_send_start( id );

    // ignore return value -> tolerate missing ACK
    platform_i2c_send_address( id, sla, PLATFORM_I2C_DIRECTION_TRANSMITTER );

    return 1;
}

static uint8_t u8g_com_esp8266_ssd_start_sequence(u8g_t *u8g)
{
    /* are we requested to set the a0 state? */
    if ( u8g->pin_list[U8G_PI_SET_A0] == 0 )
        return 1;

    /* setup bus, might be a repeated start */
    if ( do_i2c_start( ESP_I2C_ID, u8g->i2c_addr ) == 0 )
        return 0;
    if ( u8g->pin_list[U8G_PI_A0_STATE] == 0 )
    {
        // ignore return value -> tolerate missing ACK
        if ( platform_i2c_send_byte( ESP_I2C_ID, I2C_CMD_MODE ) == 0 )
            ; //return 0;
    }
    else
    {
        platform_i2c_send_byte( ESP_I2C_ID, I2C_DATA_MODE );
    }

    u8g->pin_list[U8G_PI_SET_A0] = 0;
    return 1;
}


static void lu8g_digital_write( u8g_t *u8g, uint8_t pin_index, uint8_t value )
{
    uint8_t pin;

    pin = u8g->pin_list[pin_index];
    if ( pin != U8G_PIN_NONE )
        platform_gpio_write( pin, value );
}

uint8_t u8g_com_esp8266_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
    switch(msg)
    {
    case U8G_COM_MSG_STOP:
        break;
    
    case U8G_COM_MSG_INIT:
        // we assume that the SPI interface was already initialized
        // just care for the /CS and D/C pins
        lu8g_digital_write( u8g, U8G_PI_CS, PLATFORM_GPIO_HIGH );
        platform_gpio_mode( u8g->pin_list[U8G_PI_CS], PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_FLOAT );
        platform_gpio_mode( u8g->pin_list[U8G_PI_A0], PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_FLOAT );
        break;
    
    case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
        lu8g_digital_write( u8g, U8G_PI_A0, arg_val == 0 ? PLATFORM_GPIO_LOW : PLATFORM_GPIO_HIGH );
        break;

    case U8G_COM_MSG_CHIP_SELECT:
        if (arg_val == 0)
        {
            /* disable */
            lu8g_digital_write( u8g, U8G_PI_CS, PLATFORM_GPIO_HIGH );
        }
        else
        {
            /* enable */
            //u8g_com_arduino_digital_write(u8g, U8G_PI_SCK, LOW);
            lu8g_digital_write( u8g, U8G_PI_CS, PLATFORM_GPIO_LOW );
        }
        break;
      
    case U8G_COM_MSG_RESET:
        if ( u8g->pin_list[U8G_PI_RESET] != U8G_PIN_NONE )
            lu8g_digital_write( u8g, U8G_PI_RESET, arg_val == 0 ? PLATFORM_GPIO_LOW : PLATFORM_GPIO_HIGH );
        break;
    
    case U8G_COM_MSG_WRITE_BYTE:
        platform_spi_send_recv( 1, arg_val );
        break;
    
    case U8G_COM_MSG_WRITE_SEQ:
    case U8G_COM_MSG_WRITE_SEQ_P:
        {
            register uint8_t *ptr = arg_ptr;
            while( arg_val > 0 )
            {
                platform_spi_send_recv( 1, *ptr++ );
                arg_val--;
            }
        }
        break;
    }
    return 1;
}


uint8_t u8g_com_esp8266_ssd_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
    switch(msg)
    {
    case U8G_COM_MSG_INIT:
        // we assume that the i2c bus was already initialized
        //u8g_i2c_init(u8g->pin_list[U8G_PI_I2C_OPTION]);

        break;
    
    case U8G_COM_MSG_STOP:
        break;

    case U8G_COM_MSG_RESET:
        /* Currently disabled, but it could be enable. Previous restrictions have been removed */
        /* u8g_com_arduino_digital_write(u8g, U8G_PI_RESET, arg_val); */
        break;
      
    case U8G_COM_MSG_CHIP_SELECT:
        u8g->pin_list[U8G_PI_A0_STATE] = 0;
        u8g->pin_list[U8G_PI_SET_A0] = 1;		/* force a0 to set again, also forces start condition */
        if ( arg_val == 0 )
        {
            /* disable chip, send stop condition */
            platform_i2c_send_stop( ESP_I2C_ID );
        }
        else
        {
            /* enable, do nothing: any byte writing will trigger the i2c start */
        }
        break;

    case U8G_COM_MSG_WRITE_BYTE:
        //u8g->pin_list[U8G_PI_SET_A0] = 1;
        if ( u8g_com_esp8266_ssd_start_sequence(u8g) == 0 )
            return platform_i2c_send_stop( ESP_I2C_ID ), 0;
        // ignore return value -> tolerate missing ACK
        if ( platform_i2c_send_byte( ESP_I2C_ID, arg_val) == 0 )
            ; //return platform_i2c_send_stop( ESP_I2C_ID ), 0;
        // platform_i2c_send_stop( ESP_I2C_ID );
        break;
    
    case U8G_COM_MSG_WRITE_SEQ:
    case U8G_COM_MSG_WRITE_SEQ_P:
        //u8g->pin_list[U8G_PI_SET_A0] = 1;
        if ( u8g_com_esp8266_ssd_start_sequence(u8g) == 0 )
            return platform_i2c_send_stop( ESP_I2C_ID ), 0;
        {
            register uint8_t *ptr = arg_ptr;
            while( arg_val > 0 )
            {
                // ignore return value -> tolerate missing ACK
                if ( platform_i2c_send_byte( ESP_I2C_ID, *ptr++) == 0 )
                    ; //return platform_i2c_send_stop( ESP_I2C_ID ), 0;
                arg_val--;
            }
        }
        // platform_i2c_send_stop( ESP_I2C_ID );
        break;

    case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
        u8g->pin_list[U8G_PI_A0_STATE] = arg_val;
        u8g->pin_list[U8G_PI_SET_A0] = 1;		/* force a0 to set again */
    
        break;
    }
    return 1;
}




// device destructor
static int lu8g_close_display( lua_State *L )
{
    lu8g_userdata_t *lud;

    if ((lud = get_lud( L )) == NULL)
        return 0;

    // free up allocated page buffer
    if (lud->pb.buf != NULL)
    {
        c_free( lud->pb.buf );
        lud->pb.buf = NULL;
    }

    return 0;
}


// device constructors

uint8_t u8g_dev_ssd1306_128x64_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);
// Lua: object = u8g.ssd1306_128x64_i2c( i2c_addr )
static int lu8g_ssd1306_128x64_i2c( lua_State *L )
{
    unsigned addr = luaL_checkinteger( L, 1 );

    if (addr == 0)
        return luaL_error( L, "i2c address required" );

    lu8g_userdata_t *lud = (lu8g_userdata_t *) lua_newuserdata( L, sizeof( lu8g_userdata_t ) );

    lud->u8g.i2c_addr = (uint8_t)addr;

    // Don't use the pre-defined device structure for u8g_dev_ssd1306_128x64_i2c here
    // Reason: linking the pre-defined structures allocates RAM for the device/comm structure
    //         *before* the display is constructed (especially the page buffers)
    //         this consumes heap even when the device is not used at all
#if 1
    // build device entry
    lud->dev = (u8g_dev_t){ u8g_dev_ssd1306_128x64_fn, &(lud->pb), U8G_COM_SSD_I2C };

    // populate and allocate page buffer
    // constants taken from u8g_dev_ssd1306_128x64.c:
    //                     PAGE_HEIGHT
    //                      | Height
    //                      |  |              WIDTH
    //                      |  |               |
    lud->pb = (u8g_pb_t){ { 8, 64, 0, 0, 0 }, 128, NULL };
    //
    if ((lud->pb.buf = (void *)c_zalloc(lud->pb.width)) == NULL)
        return luaL_error( L, "out of memory" );

    // and finally init device using specific interface init function
    u8g_InitI2C( LU8G, &(lud->dev), U8G_I2C_OPT_NONE);
#else
    u8g_InitI2C( LU8G, &u8g_dev_ssd1306_128x64_i2c, U8G_I2C_OPT_NONE);
#endif


    // set its metatable
    luaL_getmetatable(L, "u8g.display");
    lua_setmetatable(L, -2);

    return 1;
}

// Lua: object = u8g.ssd1306_128x64_spi( cs, dc, [res] )
static int lu8g_ssd1306_128x64_spi( lua_State *L )
{
    unsigned cs = luaL_checkinteger( L, 1 );
    if (cs == 0)
        return luaL_error( L, "CS pin required" );
    unsigned dc = luaL_checkinteger( L, 2 );
    if (dc == 0)
        return luaL_error( L, "D/C pin required" );
    unsigned res = luaL_optinteger( L, 3, U8G_PIN_NONE );

    lu8g_userdata_t *lud = (lu8g_userdata_t *) lua_newuserdata( L, sizeof( lu8g_userdata_t ) );

    // Don't use the pre-defined device structure for u8g_dev_ssd1306_128x64_spi here
    // Reason: linking the pre-defined structures allocates RAM for the device/comm structure
    //         *before* the display is constructed (especially the page buffers)
    //         this consumes heap even when the device is not used at all
#if 1
    // build device entry
    lud->dev = (u8g_dev_t){ u8g_dev_ssd1306_128x64_fn, &(lud->pb), U8G_COM_HW_SPI };

    // populate and allocate page buffer
    // constants taken from u8g_dev_ssd1306_128x64.c:
    //                     PAGE_HEIGHT
    //                      | Height
    //                      |  |              WIDTH
    //                      |  |               |
    lud->pb = (u8g_pb_t){ { 8, 64, 0, 0, 0 }, 128, NULL };
    //
    if ((lud->pb.buf = (void *)c_zalloc(lud->pb.width)) == NULL)
        return luaL_error( L, "out of memory" );

    // and finally init device using specific interface init function
    u8g_InitHWSPI( LU8G, &(lud->dev), cs, dc, res );
#else
    u8g_InitHWSPI( LU8G, &u8g_dev_ssd1306_128x64_spi, cs, dc, res );
#endif


    // set its metatable
    luaL_getmetatable(L, "u8g.display");
    lua_setmetatable(L, -2);

    return 1;
}

uint8_t u8g_dev_pcd8544_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);
// Lua: object = u8g.pcd8544_84x48( sce, dc, res )
static int lu8g_pcd8544_84x48( lua_State *L )
{
    unsigned sce = luaL_checkinteger( L, 1 );
    if (sce == 0)
        return luaL_error( L, "SCE pin required" );
    unsigned dc = luaL_checkinteger( L, 2 );
    if (dc == 0)
        return luaL_error( L, "D/C pin required" );
    unsigned res = luaL_checkinteger( L, 3 );
    if (res == 0)
        return luaL_error( L, "RES pin required" );

    lu8g_userdata_t *lud = (lu8g_userdata_t *) lua_newuserdata( L, sizeof( lu8g_userdata_t ) );

    // Don't use the pre-defined device structure for u8g_dev_pcd8544_84x48_hw_spi here
    // Reason: linking the pre-defined structures allocates RAM for the device/comm structure
    //         *before* the display is constructed (especially the page buffers)
    //         this consumes heap even when the device is not used at all
#if 1
    // build device entry
    lud->dev = (u8g_dev_t){ u8g_dev_pcd8544_fn, &(lud->pb), U8G_COM_HW_SPI };

    // populate and allocate page buffer
    // constants taken from u8g_dev_pcd8544_84x48.c:
    //                     PAGE_HEIGHT
    //                      | Height
    //                      |  |             WIDTH
    //                      |  |              |
    lud->pb = (u8g_pb_t){ { 8, 48, 0, 0, 0 }, 84, NULL };
    //
    if ((lud->pb.buf = (void *)c_zalloc(lud->pb.width)) == NULL)
        return luaL_error( L, "out of memory" );

    // and finally init device using specific interface init function
    u8g_InitHWSPI( LU8G, &(lud->dev), sce, dc, res );
#else
    u8g_InitHWSPI( LU8G, &u8g_dev_pcd8544_84x48_hw_spi, sce, dc, res );
#endif


    // set its metatable
    luaL_getmetatable(L, "u8g.display");
    lua_setmetatable(L, -2);

    return 1;
}


// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

static const LUA_REG_TYPE lu8g_display_map[] =
{
    { LSTRKEY( "begin" ),  LFUNCVAL( lu8g_begin ) },
    { LSTRKEY( "drawBitmap" ),  LFUNCVAL( lu8g_drawBitmap ) },
    { LSTRKEY( "drawBox" ),  LFUNCVAL( lu8g_drawBox ) },
    { LSTRKEY( "drawCircle" ),  LFUNCVAL( lu8g_drawCircle ) },
    { LSTRKEY( "drawDisc" ),  LFUNCVAL( lu8g_drawDisc ) },
    { LSTRKEY( "drawEllipse" ),  LFUNCVAL( lu8g_drawEllipse ) },
    { LSTRKEY( "drawFilledEllipse" ),  LFUNCVAL( lu8g_drawFilledEllipse ) },
    { LSTRKEY( "drawFrame" ),  LFUNCVAL( lu8g_drawFrame ) },
    { LSTRKEY( "drawHLine" ),  LFUNCVAL( lu8g_drawHLine ) },
    { LSTRKEY( "drawLine" ),  LFUNCVAL( lu8g_drawLine ) },
    { LSTRKEY( "drawPixel" ),  LFUNCVAL( lu8g_drawPixel ) },
    { LSTRKEY( "drawRBox" ),  LFUNCVAL( lu8g_drawRBox ) },
    { LSTRKEY( "drawRFrame" ),  LFUNCVAL( lu8g_drawRFrame ) },
    { LSTRKEY( "drawStr" ),  LFUNCVAL( lu8g_drawStr ) },
    { LSTRKEY( "drawStr90" ),  LFUNCVAL( lu8g_drawStr90 ) },
    { LSTRKEY( "drawStr180" ),  LFUNCVAL( lu8g_drawStr180 ) },
    { LSTRKEY( "drawStr270" ),  LFUNCVAL( lu8g_drawStr270 ) },
    { LSTRKEY( "drawTriangle" ),  LFUNCVAL( lu8g_drawTriangle ) },
    { LSTRKEY( "drawVLine" ),  LFUNCVAL( lu8g_drawVLine ) },
    { LSTRKEY( "drawXBM" ),  LFUNCVAL( lu8g_drawXBM ) },
    { LSTRKEY( "firstPage" ),  LFUNCVAL( lu8g_firstPage ) },
    { LSTRKEY( "getColorIndex" ),  LFUNCVAL( lu8g_getColorIndex ) },
    { LSTRKEY( "getFontAscent" ),  LFUNCVAL( lu8g_getFontAscent ) },
    { LSTRKEY( "getFontDescent" ),  LFUNCVAL( lu8g_getFontDescent ) },
    { LSTRKEY( "getFontLineSpacing" ),  LFUNCVAL( lu8g_getFontLineSpacing ) },
    { LSTRKEY( "getHeight" ),  LFUNCVAL( lu8g_getHeight ) },
    { LSTRKEY( "getMode" ),  LFUNCVAL( lu8g_getMode ) },
    { LSTRKEY( "getStrWidth" ), LFUNCVAL( lu8g_getStrWidth ) },
    { LSTRKEY( "getWidth" ),  LFUNCVAL( lu8g_getWidth ) },
    { LSTRKEY( "nextPage" ),  LFUNCVAL( lu8g_nextPage ) },
    { LSTRKEY( "setColorIndex" ),  LFUNCVAL( lu8g_setColorIndex ) },
    { LSTRKEY( "setDefaultBackgroundColor" ),  LFUNCVAL( lu8g_setDefaultBackgroundColor ) },
    { LSTRKEY( "setDefaultForegroundColor" ),  LFUNCVAL( lu8g_setDefaultForegroundColor ) },
    { LSTRKEY( "setFont" ),  LFUNCVAL( lu8g_setFont ) },
    { LSTRKEY( "setFontLineSpacingFactor" ),  LFUNCVAL( lu8g_setFontLineSpacingFactor ) },
    { LSTRKEY( "setFontPosBaseline" ),  LFUNCVAL( lu8g_setFontPosBaseline ) },
    { LSTRKEY( "setFontPosBottom" ),  LFUNCVAL( lu8g_setFontPosBottom ) },
    { LSTRKEY( "setFontPosCenter" ),  LFUNCVAL( lu8g_setFontPosCenter ) },
    { LSTRKEY( "setFontPosTop" ),  LFUNCVAL( lu8g_setFontPosTop ) },
    { LSTRKEY( "setFontRefHeightAll" ),  LFUNCVAL( lu8g_setFontRefHeightAll ) },
    { LSTRKEY( "setFontRefHeightExtendedText" ),  LFUNCVAL( lu8g_setFontRefHeightExtendedText ) },
    { LSTRKEY( "setFontRefHeightText" ),  LFUNCVAL( lu8g_setFontRefHeightText ) },
    { LSTRKEY( "setRot90" ),  LFUNCVAL( lu8g_setRot90 ) },
    { LSTRKEY( "setRot180" ),  LFUNCVAL( lu8g_setRot180 ) },
    { LSTRKEY( "setRot270" ),  LFUNCVAL( lu8g_setRot270 ) },
    { LSTRKEY( "setScale2x2" ),  LFUNCVAL( lu8g_setScale2x2 ) },
    { LSTRKEY( "sleepOff" ),  LFUNCVAL( lu8g_sleepOff ) },
    { LSTRKEY( "sleepOn" ),  LFUNCVAL( lu8g_sleepOn ) },
    { LSTRKEY( "undoRotation" ),  LFUNCVAL( lu8g_undoRotation ) },
    { LSTRKEY( "undoScale" ),  LFUNCVAL( lu8g_undoScale ) },
    { LSTRKEY( "__gc" ),  LFUNCVAL( lu8g_close_display ) },
#if LUA_OPTIMIZE_MEMORY > 0
    { LSTRKEY( "__index" ), LROVAL ( lu8g_display_map ) },
#endif
    { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE lu8g_map[] = 
{
#ifdef U8G_SSD1306_128x64_I2C
    { LSTRKEY( "ssd1306_128x64_i2c" ), LFUNCVAL ( lu8g_ssd1306_128x64_i2c ) },
#endif
#ifdef U8G_SSD1306_128x64_I2C
    { LSTRKEY( "ssd1306_128x64_spi" ), LFUNCVAL ( lu8g_ssd1306_128x64_spi ) },
#endif
#ifdef U8G_PCD8544_84x48
    { LSTRKEY( "pcd8544_84x48" ), LFUNCVAL ( lu8g_pcd8544_84x48 ) },
#endif

#if LUA_OPTIMIZE_MEMORY > 0

    // Register fonts
#undef U8G_FONT_TABLE_ENTRY
#define U8G_FONT_TABLE_ENTRY(font) { LSTRKEY( #font ), LUDATA( (void *)(u8g_ ## font) ) },
    U8G_FONT_TABLE

    // Options for circle/ ellipse drwing
    { LSTRKEY( "DRAW_UPPER_RIGHT" ), LNUMVAL( U8G_DRAW_UPPER_RIGHT ) },
    { LSTRKEY( "DRAW_UPPER_LEFT" ),  LNUMVAL( U8G_DRAW_UPPER_LEFT ) },
    { LSTRKEY( "DRAW_LOWER_RIGHT" ), LNUMVAL( U8G_DRAW_LOWER_RIGHT ) },
    { LSTRKEY( "DRAW_LOWER_LEFT" ),  LNUMVAL( U8G_DRAW_LOWER_LEFT ) },
    { LSTRKEY( "DRAW_ALL" ),         LNUMVAL( U8G_DRAW_ALL ) },

    // Display modes
    { LSTRKEY( "MODE_BW" ),       LNUMVAL( U8G_MODE_BW ) },
    { LSTRKEY( "MODE_GRAY2BIT" ), LNUMVAL( U8G_MODE_GRAY2BIT ) },

    { LSTRKEY( "__metatable" ), LROVAL( lu8g_map ) },
#endif
    { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_u8g( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
    luaL_rometatable(L, "u8g.display", (void *)lu8g_display_map);  // create metatable
    return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
    int n;
    luaL_register( L, AUXLIB_U8G, lu8g_map );

    // Set it as its own metatable
    lua_pushvalue( L, -1 );
    lua_setmetatable( L, -2 );

    // Module constants  

    // Register fonts
#undef U8G_FONT_TABLE_ENTRY
#define U8G_FONT_TABLE_ENTRY(font) MOD_REG_LUDATA( L, #font, (void *)(u8g_ ## font) );
    U8G_FONT_TABLE

    // Options for circle/ ellipse drawing
    MOD_REG_NUMBER( L, "DRAW_UPPER_RIGHT", U8G_DRAW_UPPER_RIGHT );
    MOD_REG_NUMBER( L, "DRAW_UPPER_LEFT",  U8G_DRAW_UPPER_RIGHT );
    MOD_REG_NUMBER( L, "DRAW_LOWER_RIGHT", U8G_DRAW_UPPER_RIGHT );
    MOD_REG_NUMBER( L, "DRAW_LOWER_LEFT",  U8G_DRAW_UPPER_RIGHT );

    // Display modes
    MOD_REG_NUMBER( L, "MODE_BW",       U8G_MODE_BW );
    MOD_REG_NUMBER( L, "MODE_GRAY2BIT", U8G_MODE_BW );

    // create metatable
    luaL_newmetatable(L, "u8g.display");
    // metatable.__index = metatable
    lua_pushliteral(L, "__index");
    lua_pushvalue(L,-2);
    lua_rawset(L,-3);
    // Setup the methods inside metatable
    luaL_register( L, NULL, u8g_display_map );

    return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
