/**
* @Licence WTFPL
*/

#pragma once
#include <stdint.h>


#define _FTOA_TOO_LARGE -2  // |input| > 2147483520
#define _FTOA_TOO_SMALL -1  // |input| < 0.0000001

int ftoa(float f, char *p, uint8_t precision);