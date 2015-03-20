#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef ICACHE_IRAM
#define ICACHE_IRAM_ATTR __attribute__((section(".text")))
#else
#define ICACHE_IRAM_ATTR
#endif /* ICACHE_IRAM */

#endif
