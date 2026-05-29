/*
 * telemetry_data.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */
#include "telemetry_data.h"


const char* dht_status_to_string(DhtStatus_t status) {
    switch (status) {
        case DHT_OK:              return "DHT_OK";
        case DHT_ERROR: 		  return "DHT_ERROR";
        default:                  return "DHT_UNKNOWN_STATUS";
    }
}
const char* load_status_to_string(LoadStatus_t status) {
    switch (status) {
    	case LOAD_OK:              return "LOAD_OK";
        case LOAD_ADC_ERROR:   return "LOAD_ADC_ERROR";
        default:                  return "LOAD_UNKNOWN_STATUS";
    }
}
const char* system_status_to_string(SystemStatus_t status) {
    switch (status) {
    	case SYSTEM_OK:             return "SYSTEM_OK";
        case SYSTEM_WARNING:   		return "SYSTEM_WARNING";
        case SYSTEM_ERROR: 			return "SYSTEM_ERROR";
        default:                  	return "SYSTEM_UNKNOWN_STATUS";
    }
}
