/*
 * settings.h
 *
 *  Created on: 6 апр. 2018 г.
 *      Author: Satty
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "stdint.h"

volatile extern const uint8_t SlaveId __attribute__((__section__(".user_data")));

#endif /* SETTINGS_H_ */
