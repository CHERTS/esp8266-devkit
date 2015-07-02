#ifndef INCLUDE_ROUTINES_H_
#define INCLUDE_ROUTINES_H_

#define os_malloc   pvPortMalloc
#define os_free     vPortFree
#define os_zalloc   pvPortZalloc

void do_global_ctors(void);

#endif /* INCLUDE_ROUTINES_H_ */





