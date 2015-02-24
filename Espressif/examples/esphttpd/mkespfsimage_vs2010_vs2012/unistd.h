#ifndef _UNISTD_H
#define _UNISTD_H        1

/* This file intended to serve as a drop-in replacement for 
 *  *  unistd.h on Windows
 *   *  Please add functionality as neeeded 
 *    */

#include <stdlib.h>
#include <io.h>
#include <getopt.h> /* getopt from: http://www.pwilson.net/sample.html. */

#define srandom srand
#define random rand

/*
const W_OK = 2;
const R_OK = 4;
*/

#define access _access
#define ftruncate _chsize

#define ssize_t int

#endif /* unistd.h  */
