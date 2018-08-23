/*
 * defs.h
 *
 *  Created on: 6 апр. 2018 г.
 *      Author: Satty
 */

#ifndef DEFS_H_
#define DEFS_H_

#include "stdint.h"

#define BYTES_2_SHORT(hbyte, lbyte) (hbyte | (uint16_t)lbyte << 8)

void volmemcpy (void *dest, volatile const void *src, uint16_t len);

#endif /* DEFS_H_ */
