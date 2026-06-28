/*
 * telemetry_data.h
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#ifndef INC_TELEMETRY_DATA_H_
#define INC_TELEMETRY_DATA_H_
#include <stdint.h>
#include <stdbool.h>
typedef enum {
    DHT_OK,
	DHT_ERROR
} DhtStatus_t;

typedef enum {
	LOAD_OK,
	LOAD_ADC_ERROR
} LoadStatus_t;

typedef enum {
	SYSTEM_OK,
	SYSTEM_WARNING,
	SYSTEM_ERROR
} SystemStatus_t;


typedef struct
{
	int temperature;
	int humidity;
	int load;
	uint32_t fan_rpm;
	int32_t vibrationX_mg;
	int32_t vibrationY_mg;
	int32_t vibrationZ_mg;
	int32_t vibration_level_mg;
	bool emergency_button;
	DhtStatus_t dhtStatus;
	LoadStatus_t loadStatus;
	SystemStatus_t systemStatus;
} TelemetryData;


const char* dht_status_to_string(DhtStatus_t status);

const char* load_status_to_string(LoadStatus_t status);

const char* system_status_to_string(SystemStatus_t status);
#endif /* INC_TELEMETRY_DATA_H_ */
