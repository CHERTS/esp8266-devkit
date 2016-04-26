#ifndef __SDIO_SLAVE_H__
#define __SDIO_SLAVE_H__
#include "c_types.h"
#include "user_interface.h"

#define RX_AVAILIBLE 2
#define TX_AVAILIBLE 1
#define INIT_STAGE	 0

void sdio_slave_init(void);

int32 sdio_load_data(const uint8* data,uint32 len);
typedef void (*sdio_recv_data_callback_t)(uint8* data,uint32 len);

bool sdio_register_recv_cb(sdio_recv_data_callback_t cb);
#endif
