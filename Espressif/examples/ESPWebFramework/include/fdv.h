/*
# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/

#ifndef _FDV_H_
#define _FDV_H_


// disable macros like "read"
#ifndef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS 0
#endif


extern "C"
{
    #include "esp_common.h"    
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
}


// used for data
#define FLASHMEM __attribute__((aligned(4))) __attribute__((section(".irom.text")))

// used for parameters
#define FSTR(s) (__extension__({static const char __c[] FLASHMEM = (s); &__c[0];}))

// used for functions
#define FUNC_FLASHMEM __attribute__((section(".irom0.text")))

// used for methods
#define MTD_FLASHMEM __attribute__((section(".irom1.text")))


#include "fdvprintf.h"
#include "fdvdebug.h"
#include "fdvsync.h"
#include "fdvtask.h"
#include "fdvflash.h"
#include "fdvutils.h"
#include "fdvstrings.h"
#include "fdvcollections.h"
#include "fdvserial.h"
#include "fdvnetwork.h"





#endif