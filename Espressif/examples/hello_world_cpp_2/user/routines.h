/*
 * routines.h
 *
 *  Created on: 18 ���. 2015 �.
 *      Author: Alex
 */

#pragma once

#define os_malloc   pvPortMalloc
#define os_free     vPortFree
#define os_zalloc   pvPortZalloc

void do_global_ctors(void);




