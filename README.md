# STM32 Machine I/O Node

STM32 firmware for a small machine monitoring and control node.

The STM32 reads sensors, manages machine state, controls status LEDs, and communicates with a Raspberry Pi edge gateway over UART.

## Project Goal

This project demonstrates the microcontroller side of an embedded machine monitoring system.

The STM32 acts as the machine I/O node. It provides telemetry and accepts control commands from the Raspberry Pi gateway.

## Related Repository

The Raspberry Pi / Yocto / ROS2 gateway used with this firmware is available here:

```text
https://github.com/anasmansouri/machine-monitoring-edge-gateway
```

## System Architecture

```text
+----------------------+
| STM32 Machine I/O    |
|                      |
| - Sensor reading     |
| - Machine state      |
| - Fault handling     |
| - Status LEDs        |
| - UART protocol      |
+----------+-----------+
           |
           | UART
           |
+----------v-----------+
| Raspberry Pi Gateway |
| Yocto Linux + ROS2   |
+----------------------+
```

## Hardware

* STM32 Nucleo board
* DHT11 / KY-015 temperature and humidity sensor
* Load sensor / potentiometer
* Status LEDs
* UART connection to Raspberry Pi

## Main Features

* UART command-response protocol
* Machine state management
* Sensor status monitoring
* Load threshold handling
* Fault detection
* Status LED control
* Runtime state evaluation

## Machine States

```text
MACHINE_STATE_IDLE
MACHINE_STATE_RUNNING
MACHINE_STATE_WARNING
MACHINE_STATE_FAULT
```

## Fault Codes

```text
FAULT_NONE
FAULT_DHT_SENSOR_ERROR
FAULT_LOAD_SENSOR_ERROR
FAULT_LOAD_TOO_HIGH
FAULT_OVERTEMPERATURE
```

## UART Commands

The STM32 receives commands from the Raspberry Pi.

```text
PING
GET_STATUS
START_MACHINE
STOP_MACHINE
RESET_FAULT
SET_LOAD_THRESHOLD:WARN=<value>;FAULT=<value>
```

## UART Responses

Example responses:

```text
ACK:PING
ACK:START_MACHINE
ACK:STOP_MACHINE
ACK:RESET_FAULT
ACK:SET_LOAD_THRESHOLD
NACK:UNKNOWN_CMD
NACK:FAULT_ACTIVE
NACK:NOT_IDLE
```

Example status message:

```text
STATUS:TEMP=24;HUM=50;LOAD=35;STATE=MACHINE_STATE_RUNNING;FAULT=FAULT_NONE;OPERATING_MODE=AUTO_MODE;DHT_STATUS=DHT_OK;LOAD_STATUS=LOAD_OK
```

## Command Behavior

### PING

Used by the Raspberry Pi gateway to check if the STM32 is alive.

Response:

```text
ACK:PING
```

### GET_STATUS

Returns current telemetry and machine status.

Response example:

```text
STATUS:TEMP=24;HUM=50;LOAD=35;STATE=MACHINE_STATE_RUNNING;FAULT=FAULT_NONE;OPERATING_MODE=AUTO_MODE;DHT_STATUS=DHT_OK;LOAD_STATUS=LOAD_OK
```

### START_MACHINE

Starts the machine if the system is safe.

The command is rejected if:

* The machine is already running
* A fault is active
* A sensor error exists
* Load or temperature is above fault threshold

### STOP_MACHINE

Stops the machine when it is not in fault state.

If the machine is in fault state, the fault is kept latched and must be cleared using `RESET_FAULT`.

### RESET_FAULT

Clears the fault only if current sensor values are safe.

### SET_LOAD_THRESHOLD

Updates warning and fault thresholds for load.

Example:

```text
SET_LOAD_THRESHOLD:WARN=60;FAULT=85
```

Valid values:

```text
0 <= warning < fault <= 100
```

## LED Behavior

```text
IDLE     -> LEDs off
RUNNING  -> Green LED
WARNING  -> Yellow LED
FAULT    -> Red LED
```

## Runtime Behavior

The firmware continuously evaluates the machine state.

If the machine is running and a fault condition appears, the state changes to:

```text
MACHINE_STATE_FAULT
```

Faults are latched. They are not cleared automatically.

The Raspberry Pi must send:

```text
RESET_FAULT
```

to clear the fault after the system becomes safe again.

## UART Settings

```text
Baud rate: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```
## Skills Demonstrated

* STM32 firmware development
* Embedded C
* UART communication
* Sensor integration
* Command-response protocol design
* Fault handling
* Machine state management
* GPIO LED control
* Integration with Embedded Linux gateway
