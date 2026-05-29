/*
 * dht11.h
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_
#include "main.h"


void DHT11_Init(void);
int DHT11_Read(int *temperature, int *humidity);

#endif /* INC_DHT11_H_ */
