/**
* \file
*		Ring Buffer library
*/

#include "ringbuf.h"
#include "osapi.h"

/**
* \brief init a RINGBUF object
* \param r pointer to a RINGBUF object
* \param buf pointer to a byte array
* \param size size of buf
* \return 0 if successfull, otherwise failed
*/
uint16 ICACHE_FLASH_ATTR RINGBUF_Init(RINGBUF *r, uint8* buf, uint32 size)
{
	if(r == NULL || buf == NULL || size < 2) return -1;
	
	r->p_o = r->p_r = r->p_w = buf;
	r->fill_cnt = 0;
	r->size = size;
	
	return 0;
}
/**
* \brief put a character into ring buffer
* \param r pointer to a ringbuf object
* \param c character to be put
* \return 0 if successfull, otherwise failed
*/
uint16 RINGBUF_Put(RINGBUF *r, uint8 c)
{
	if(r->fill_cnt>=r->size)return -1;		// ring buffer is full, this should be atomic operation
	

	r->fill_cnt++;							// increase filled slots count, this should be atomic operation

	
	*r->p_w++ = c;							// put character into buffer
	
	if(r->p_w >= r->p_o + r->size)			// rollback if write pointer go pass
		r->p_w = r->p_o;					// the physical boundary
	
	return 0;
}
/**
* \brief get a character from ring buffer
* \param r pointer to a ringbuf object
* \param c read character
* \return 0 if successfull, otherwise failed
*/
uint16 ICACHE_FLASH_ATTR RINGBUF_Get(RINGBUF *r, uint8* c)
{
	if(r->fill_cnt<=0)return -1;				// ring buffer is empty, this should be atomic operation
	

	r->fill_cnt--;								// decrease filled slots count

	
	*c = *r->p_r++;								// get the character out
	
	if(r->p_r >= r->p_o + r->size)				// rollback if write pointer go pass
		r->p_r = r->p_o;						// the physical boundary
	
	return 0;
}

bool ICACHE_FLASH_ATTR RINGBUF_Check(RINGBUF *r,uint16 data_len)
{
	//os_printf("r->size - r->fill_cnt:data_len ; %d:%d \r\n",r->size - r->fill_cnt,data_len);

	if( r->size - r->fill_cnt >=data_len) 
		return true;
	else 
		return false;
}

sint8 ICACHE_FLASH_ATTR
	RINGBUF_Pull(RINGBUF* r,uint8* buf,uint16 length)
{
	if(r->fill_cnt < length) return -1;

    int i;
	uint8* pdata = buf;
	for(i=0;i<length;i++){
		RINGBUF_Get(r,pdata);
		pdata++;
	}
	return 0;
}

sint8 ICACHE_FLASH_ATTR
	RINGBUF_PullRaw(RINGBUF* r,uint8* buf,uint16 length,uint16 offset)
{
	if(r->fill_cnt < length) return -1;

    int i;
	uint8* volatile p_r = r->p_r;
	volatile uint32  fcnt = r->fill_cnt;

	uint8 tmp;
    for(i=0;i<offset;i++){
        RINGBUF_Get(r,&tmp);
    }

	uint8* pdata = buf;
	for(i=0;i<length;i++){
		RINGBUF_Get(r,pdata);
		pdata++;
	}

	r->p_r=p_r;
	r->fill_cnt = fcnt;
	return 0;
}




sint8 ICACHE_FLASH_ATTR
	RINGBUF_Push(RINGBUF* r,uint8* buf,uint16 length)
{
	if((r->size - r->fill_cnt) < length) return -1;

	int i;
	for(i=0;i<length;i++){
		RINGBUF_Put(r,*(buf++));
	}
}
sint8 ICACHE_FLASH_ATTR RINGBUF_Drop(RINGBUF* r,uint16 length)
{
	if( r->fill_cnt < length){
		r->p_r = r->p_w;
		r->fill_cnt = r->size;
		return 0;
	}
	
	int i; 
	for(i=0;i<length;i++){
        r->fill_cnt--;								// increase filled slots count
        //*c = *r->p_r++; 	
        r->p_r++; 	                                // move p_r
        if(r->p_r >= r->p_o + r->size){				// rollback if write pointer go pass
        	r->p_r = r->p_o;						// the physical boundary
        }
	}
	return 0;
}
