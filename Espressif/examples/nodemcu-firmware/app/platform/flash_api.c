/******************************************************************************
 * Flash api for NodeMCU
 * NodeMCU Team
 * 2014-12-31
*******************************************************************************/
#include "user_config.h"
#include "flash_api.h"
#include "spi_flash.h"
#include "c_stdio.h"

static volatile const uint8_t flash_init_data[128] ICACHE_STORE_ATTR ICACHE_RODATA_ATTR =
{
    0x05, 0x00, 0x04, 0x02, 0x05, 0x05, 0x05, 0x02, 0x05, 0x00, 0x04, 0x05, 0x05, 0x04, 0x05, 0x05,
    0x04, 0xFE, 0xFD, 0xFF, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xE1, 0x0A, 0xFF, 0xFF, 0xF8, 0x00,
    0xF8, 0xF8, 0x52, 0x4E, 0x4A, 0x44, 0x40, 0x38, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xE1, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x93, 0x43, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint32_t flash_detect_size_byte(void)
{
#define FLASH_BUFFER_SIZE_DETECT 32
    uint32_t dummy_size = FLASH_SIZE_256KBYTE;
    uint8_t data_orig[FLASH_BUFFER_SIZE_DETECT] ICACHE_STORE_ATTR = {0};
    uint8_t data_new[FLASH_BUFFER_SIZE_DETECT] ICACHE_STORE_ATTR = {0};
    if (SPI_FLASH_RESULT_OK == flash_safe_read(0, (uint32 *)data_orig, FLASH_BUFFER_SIZE_DETECT))
    {
        dummy_size = FLASH_SIZE_256KBYTE;
        while ((dummy_size < FLASH_SIZE_16MBYTE) &&
                (SPI_FLASH_RESULT_OK == flash_safe_read(dummy_size, (uint32 *)data_new, FLASH_BUFFER_SIZE_DETECT)) &&
                (0 != os_memcmp(data_orig, data_new, FLASH_BUFFER_SIZE_DETECT))
              )
        {
            dummy_size *= 2;
        }
    };
    return dummy_size;
#undef FLASH_BUFFER_SIZE_DETECT
}

uint32_t flash_safe_get_size_byte(void)
{
    static uint32_t flash_size = 0;
    if (flash_size == 0)
    {
        flash_size = flash_detect_size_byte();
    }
    return flash_size;
}

uint16_t flash_safe_get_sec_num(void)
{
    return (flash_safe_get_size_byte() / (SPI_FLASH_SEC_SIZE));
}

SpiFlashOpResult flash_safe_read(uint32 src_addr, uint32 *des_addr, uint32 size)
{
    SpiFlashOpResult result = SPI_FLASH_RESULT_ERR;
    FLASH_SAFEMODE_ENTER();
    result = spi_flash_read(src_addr, (uint32 *) des_addr, size);
    FLASH_SAFEMODE_LEAVE();
    return result;
}

SpiFlashOpResult flash_safe_write(uint32 des_addr, uint32 *src_addr, uint32 size)
{
    SpiFlashOpResult result = SPI_FLASH_RESULT_ERR;
    FLASH_SAFEMODE_ENTER();
    result = spi_flash_write(des_addr, src_addr, size);
    FLASH_SAFEMODE_LEAVE();
    return result;
}

SpiFlashOpResult flash_safe_erase_sector(uint16 sec)
{
    SpiFlashOpResult result = SPI_FLASH_RESULT_ERR;
    FLASH_SAFEMODE_ENTER();
    result = spi_flash_erase_sector(sec);
    FLASH_SAFEMODE_LEAVE();
    return result;
}

SPIFlashInfo flash_rom_getinfo(void)
{
    volatile SPIFlashInfo spi_flash_info ICACHE_STORE_ATTR;
    // Don't use it before cache read disabled
    // FLASH_DISABLE_CACHE();
    // spi_flash_info = *((SPIFlashInfo *)(FLASH_ADDRESS_START_MAP));
    // FLASH_ENABLE_CACHE();
    // Needn't safe mode.
    spi_flash_read(0, (uint32 *)(& spi_flash_info), sizeof(spi_flash_info));
    return spi_flash_info;
}

uint8_t flash_rom_get_size_type(void)
{
    return flash_rom_getinfo().size;
}

uint32_t flash_rom_get_size_byte(void)
{
    static uint32_t flash_size = 0;
    if (flash_size == 0)
    {
        switch (flash_rom_getinfo().size)
        {
        case SIZE_2MBIT:
            // 2Mbit, 256kByte
            flash_size = 256 * 1024;
            break;
        case SIZE_4MBIT:
            // 4Mbit, 512kByte
            flash_size = 512 * 1024;
            break;
        case SIZE_8MBIT:
            // 8Mbit, 1MByte
            flash_size = 1 * 1024 * 1024;
            break;
        case SIZE_16MBIT:
            // 16Mbit, 2MByte
            flash_size = 2 * 1024 * 1024;
            break;
        case SIZE_32MBIT:
            // 32Mbit, 4MByte
            flash_size = 4 * 1024 * 1024;
            break;
        case SIZE_64MBIT:
            // 64Mbit, 8MByte
            flash_size = 8 * 1024 * 1024;
            break;
        case SIZE_128MBIT:
            // 128Mbit, 16MByte
            flash_size = 16 * 1024 * 1024;
            break;
        default:
            // Unknown flash size, fall back mode.
            flash_size = 512 * 1024;
            break;
        }
    }
    return flash_size;
}

bool flash_rom_set_size_type(uint8_t size)
{
    // Dangerous, here are dinosaur infested!!!!!
    // Reboot required!!!
    // If you don't know what you're doing, your nodemcu may turn into stone ...
    NODE_DBG("\nBEGIN SET FLASH HEADER\n");
    uint8_t data[SPI_FLASH_SEC_SIZE] ICACHE_STORE_ATTR;
    if (SPI_FLASH_RESULT_OK == spi_flash_read(0, (uint32 *)data, SPI_FLASH_SEC_SIZE))
    {
        ((SPIFlashInfo *)(&data[0]))->size = size;
        if (SPI_FLASH_RESULT_OK == spi_flash_erase_sector(0 * SPI_FLASH_SEC_SIZE))
        {
            NODE_DBG("\nERASE SUCCESS\n");
        }
        if (SPI_FLASH_RESULT_OK == spi_flash_write(0, (uint32 *)data, SPI_FLASH_SEC_SIZE))
        {
            NODE_DBG("\nWRITE SUCCESS, %u\n", size);
        }
    }
    NODE_DBG("\nEND SET FLASH HEADER\n");
    return true;
}

bool flash_rom_set_size_byte(uint32_t size)
{
    // Dangerous, here are dinosaur infested!!!!!
    // Reboot required!!!
    // If you don't know what you're doing, your nodemcu may turn into stone ...
    bool result = true;
    uint32_t flash_size = 0;
    switch (size)
    {
    case 256 * 1024:
        // 2Mbit, 256kByte
        flash_size = SIZE_2MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    case 512 * 1024:
        // 4Mbit, 512kByte
        flash_size = SIZE_4MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    case 1 * 1024 * 1024:
        // 8Mbit, 1MByte
        flash_size = SIZE_8MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    case 2 * 1024 * 1024:
        // 16Mbit, 2MByte
        flash_size = SIZE_16MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    case 4 * 1024 * 1024:
        // 32Mbit, 4MByte
        flash_size = SIZE_32MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    case 8 * 1024 * 1024:
        // 64Mbit, 8MByte
        flash_size = SIZE_64MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    case 16 * 1024 * 1024:
        // 128Mbit, 16MByte
        flash_size = SIZE_128MBIT;
        flash_rom_set_size_type(flash_size);
        break;
    default:
        // Unknown flash size.
        result = false;
        break;
    }
    return result;
}

uint16_t flash_rom_get_sec_num(void)
{
    //static uint16_t sec_num = 0;
    // return flash_rom_get_size_byte() / (SPI_FLASH_SEC_SIZE);
    // c_printf("\nflash_rom_get_size_byte()=%d\n", ( flash_rom_get_size_byte() / (SPI_FLASH_SEC_SIZE) ));
    // if( sec_num == 0 )
    //{
    //    sec_num = 4 * 1024 * 1024 / (SPI_FLASH_SEC_SIZE);
    //}
    //return sec_num;
    return ( flash_rom_get_size_byte() / (SPI_FLASH_SEC_SIZE) );
}

uint8_t flash_rom_get_mode(void)
{
    SPIFlashInfo spi_flash_info = flash_rom_getinfo();
    switch (spi_flash_info.mode)
    {
    // Reserved for future use
    case MODE_QIO:
        break;
    case MODE_QOUT:
        break;
    case MODE_DIO:
        break;
    case MODE_DOUT:
        break;
    }
    return spi_flash_info.mode;
}

uint32_t flash_rom_get_speed(void)
{
    uint32_t speed = 0;
    SPIFlashInfo spi_flash_info = flash_rom_getinfo();
    switch (spi_flash_info.speed)
    {
    case SPEED_40MHZ:
        // 40MHz
        speed = 40000000;
        break;
    case SPEED_26MHZ:
        //26.7MHz
        speed = 26700000;
        break;
    case SPEED_20MHZ:
        // 20MHz
        speed = 20000000;
        break;
    case SPEED_80MHZ:
        //80MHz
        speed = 80000000;
        break;
    }
    return speed;
}

bool flash_rom_set_speed(uint32_t speed)
{
    // Dangerous, here are dinosaur infested!!!!!
    // Reboot required!!!
    // If you don't know what you're doing, your nodemcu may turn into stone ...
    NODE_DBG("\nBEGIN SET FLASH HEADER\n");
    uint8_t data[SPI_FLASH_SEC_SIZE] ICACHE_STORE_ATTR;
    uint8_t speed_type = SPEED_40MHZ;
    if (speed < 26700000)
    {
        speed_type = SPEED_20MHZ;
    }
    else if (speed < 40000000)
    {
        speed_type = SPEED_26MHZ;
    }
    else if (speed < 80000000)
    {
        speed_type = SPEED_40MHZ;
    }
    else if (speed >= 80000000)
    {
        speed_type = SPEED_80MHZ;
    }
    if (SPI_FLASH_RESULT_OK == spi_flash_read(0, (uint32 *)data, SPI_FLASH_SEC_SIZE))
    {
        ((SPIFlashInfo *)(&data[0]))->speed = speed_type;
        if (SPI_FLASH_RESULT_OK == spi_flash_erase_sector(0 * SPI_FLASH_SEC_SIZE))
        {
            NODE_DBG("\nERASE SUCCESS\n");
        }
        if (SPI_FLASH_RESULT_OK == spi_flash_write(0, (uint32 *)data, SPI_FLASH_SEC_SIZE))
        {
            NODE_DBG("\nWRITE SUCCESS, %u\n", speed_type);
        }
    }
    NODE_DBG("\nEND SET FLASH HEADER\n");
    return true;
}

bool flash_init_data_written(void)
{
    // FLASH SEC - 4
    uint32_t data[2] ICACHE_STORE_ATTR;
#if defined(FLASH_SAFE_API)
    if (SPI_FLASH_RESULT_OK == flash_safe_read((flash_safe_get_sec_num() - 4) * SPI_FLASH_SEC_SIZE, (uint32 *)data, sizeof(data)))
#else
    if (SPI_FLASH_RESULT_OK == spi_flash_read((flash_rom_get_sec_num() - 4) * SPI_FLASH_SEC_SIZE, (uint32 *)data, sizeof(data)))
#endif // defined(FLASH_SAFE_API)
    {
        if (data[0] == 0xFFFFFFFF && data[1] == 0xFFFFFFFF)
        {
            return false;
        }
    }
    return true;
}

bool flash_init_data_default(void)
{
    // FLASH SEC - 4
    // Dangerous, here are dinosaur infested!!!!!
    // Reboot required!!!
    // It will init system data to default!
    bool result = false;
#if defined(FLASH_SAFE_API)
    if (SPI_FLASH_RESULT_OK == flash_safe_erase_sector((flash_safe_get_sec_num() - 4)))
    {
        if (SPI_FLASH_RESULT_OK == flash_safe_write((flash_safe_get_sec_num() - 4) * SPI_FLASH_SEC_SIZE, (uint32 *)flash_init_data, 128))
        {
            result = true;
        }
    }
#else
    if (SPI_FLASH_RESULT_OK == spi_flash_erase_sector((flash_rom_get_sec_num() - 4)))
    {
        if (SPI_FLASH_RESULT_OK == spi_flash_write((flash_rom_get_sec_num() - 4) * SPI_FLASH_SEC_SIZE, (uint32 *)flash_init_data, 128))
        {
            result = true;
        }
    }
#endif // defined(FLASH_SAFE_API)
    return result;
}

bool flash_init_data_blank(void)
{
    // FLASH SEC - 2
    // Dangerous, here are dinosaur infested!!!!!
    // Reboot required!!!
    // It will init system config to blank!
    bool result = false;
#if defined(FLASH_SAFE_API)
    if ((SPI_FLASH_RESULT_OK == flash_safe_erase_sector((flash_safe_get_sec_num() - 2))) &&
            (SPI_FLASH_RESULT_OK == flash_safe_erase_sector((flash_safe_get_sec_num() - 1))))
#else
    if ((SPI_FLASH_RESULT_OK == spi_flash_erase_sector((flash_rom_get_sec_num() - 2))) &&
            (SPI_FLASH_RESULT_OK == spi_flash_erase_sector((flash_rom_get_sec_num() - 1))))
#endif // defined(FLASH_SAFE_API)
    {
        result = true;
    }

    return result ;
}

bool flash_self_destruct(void)
{
    // Dangerous, Erase your flash. Good bye!
    SPIEraseChip();
    return true;
}

uint8_t byte_of_aligned_array(const uint8_t *aligned_array, uint32_t index)
{
    if ( (((uint32_t)aligned_array) % 4) != 0 )
    {
        NODE_DBG("aligned_array is not 4-byte aligned.\n");
        return 0;
    }
    volatile uint32_t v = ((uint32_t *)aligned_array)[ index / 4 ];
    uint8_t *p = (uint8_t *) (&v);
    return p[ (index % 4) ];
}

uint16_t word_of_aligned_array(const uint16_t *aligned_array, uint32_t index)
{
    if ( (((uint32_t)aligned_array) % 4) != 0 )
    {
        NODE_DBG("aligned_array is not 4-byte aligned.\n");
        return 0;
    }
    volatile uint32_t v = ((uint32_t *)aligned_array)[ index / 2 ];
    uint16_t *p = (uint16_t *) (&v);
    return (index % 2 == 0) ? p[ 0 ] : p[ 1 ];
    // return p[ (index % 2) ]; // -- why error???
    // (byte_of_aligned_array((uint8_t *)aligned_array, index * 2 + 1) << 8) | byte_of_aligned_array((uint8_t *)aligned_array, index * 2);
}

// uint8_t flash_rom_get_checksum(void)
// {
//     // SPIFlashInfo spi_flash_info ICACHE_STORE_ATTR = flash_rom_getinfo();
//     // uint32_t address = sizeof(spi_flash_info) + spi_flash_info.segment_size;
//     // uint32_t address_aligned_4bytes = (address + 3) & 0xFFFFFFFC;
//     // uint8_t buffer[64] = {0};
//     // spi_flash_read(address, (uint32 *) buffer, 64);
//     // uint8_t i = 0;
//     // c_printf("\nBEGIN DUMP\n");
//     // for (i = 0; i < 64; i++)
//     // {
//     //     c_printf("%02x," , buffer[i]);
//     // }
//     // i = (address + 0x10) & 0x10 - 1;
//     // c_printf("\nSIZE:%d CHECK SUM:%02x\n", spi_flash_info.segment_size, buffer[i]);
//     // c_printf("\nEND DUMP\n");
//     // return buffer[0];
//     return 0;
// }

// uint8_t flash_rom_calc_checksum(void)
// {
//     return 0;
// }