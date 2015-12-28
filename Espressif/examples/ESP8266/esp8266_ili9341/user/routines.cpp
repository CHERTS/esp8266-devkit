
#include <c_types.h>
#include "cpp_routines/routines.h"

extern "C"
{
#include <osapi.h>
   void *pvPortMalloc( size_t xWantedSize );
   void vPortFree( void *pv );
   void *pvPortZalloc(size_t size);
}


void *operator new(size_t size)
{
   return os_malloc(size);
}

void *operator new[](size_t size)
{
   return os_malloc(size);
}

void operator delete(void * ptr)
{
   os_free(ptr);
}

void operator delete[](void * ptr)
{
   os_free(ptr);
}

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));
extern "C" void abort()
{
   while(true); // enter an infinite loop and get reset by the WDT
}

void __cxa_pure_virtual(void) {
   abort();
}

void __cxa_deleted_virtual(void) {
   abort();
}

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

void do_global_ctors(void)
{
   void (**p)(void);
   for (p = &__init_array_start; p != &__init_array_end; ++p)
   {
      (*p)();
   }
}

