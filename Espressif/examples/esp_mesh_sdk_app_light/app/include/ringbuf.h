#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#include "os_type.h"
//#include <stdlib.h>
//#include "typedef.h"
#include "c_types.h"

typedef struct{
	uint8* p_o;				/**< Original pointer */
	uint8* volatile p_r;		/**< Read pointer */
	uint8* volatile p_w;		/**< Write pointer */
	volatile uint32 fill_cnt;	/**< Number of filled slots */
	uint32 size;				/**< Buffer size */
}RINGBUF;

uint16 RINGBUF_Init(RINGBUF *r, uint8* buf, uint32 size);
uint16 RINGBUF_Put(RINGBUF *r, uint8 c);
uint16 RINGBUF_Get(RINGBUF *r, uint8* c);
bool RINGBUF_Check(RINGBUF *r,uint16 data_len);
sint8 RINGBUF_Pull(RINGBUF* r,uint8* buf,uint16 length);
sint8 RINGBUF_PullRaw(RINGBUF* r,uint8* buf,uint16 length,uint16 offset);
sint8 RINGBUF_Push(RINGBUF* r,uint8* buf,uint16 length);
sint8 RINGBUF_Drop(RINGBUF* r,uint16 length);



#endif
