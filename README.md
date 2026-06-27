# STM32 Machine I/O Node

STM32 firmware for a small industrial-style machine monitoring and control node.

The STM32 reads sensors, measures fan feedback, manages machine state, controls status LEDs, and communicates with a Raspberry Pi edge gateway over UART.

## Project Goal

This project demonstrates the microcontroller side of an embedded machine monitoring system.

The STM32 acts as the machine I/O node. It collects telemetry from real inputs, evaluates basic machine state and fault conditions, and exposes the latest machine snapshot to a Raspberry Pi gateway through a simple UART command-response protocol.

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
| - DHT11 telemetry    |
| - Load input         |
| - ADXL345 vibration  |
| - Fan RPM feedback   |
| - Emergency input    |
| - Machine state      |
| - Fault handling     |
| - Status LEDs        |
| - UART protocol      |
+----------+-----------+
           |
           | USART1 / UART
           |
+----------v-----------+
| Raspberry Pi Gateway |
| Yocto Linux + ROS2   |
+----------------------+
```

End-to-end data flow:

```text
STM32 sensors
  -> TelemetryTask
  -> latestTelemetry
  -> GET_STATUS response over USART1
  -> Raspberry Pi edge-gateway
  -> Unix socket JSON
  -> ROS2 /machine/telemetry
```

## Hardware

* STM32 Nucleo board
* DHT11 / KY-015 temperature and humidity sensor
* Load sensor / potentiometer
* ADXL345 vibration sensor over I2C
* 4-pin PWM fan with tach/RPM feedback
* Emergency stop / emergency button input
* Status LEDs
* UART connection to Raspberry Pi

## Main Features

* FreeRTOS-based task architecture
* UART command-response protocol on USART1
* DHT11 temperature and humidity telemetry
* Load percentage telemetry from ADC
* ADXL345 vibration telemetry on X/Y/Z axes
* Moving average filtering for vibration values
* Fan RPM measurement from tach pulses
* Emergency button state reporting
* Machine state management
* Load and temperature threshold handling
* Fault detection and latched fault behavior
* Status LED control
* Runtime state evaluation
* Integration with Raspberry Pi Yocto/ROS2 gateway

## Firmware Tasks

```text
UartRxTask
  Receives UART command bytes from Raspberry Pi on USART1 and pushes complete commands into a queue.

CommandTask
  Consumes queued commands, reads the latest telemetry snapshot, calls the protocol handler, and sends the response over USART1.

TelemetryTask
  Reads sensors, updates latestTelemetry, calculates fan RPM, reads emergency input, and evaluates runtime machine state.

DefaultTask
  Currently kept minimal. Early hardware debug code is commented out here.
```

## Telemetry Data

The main telemetry structure contains:

```c
typedef struct
{
    int temperature;
    int humidity;
    int load;
    uint32_t fan_rpm;
    int32_t vibrationX_mg;
    int32_t vibrationY_mg;
    int32_t vibrationZ_mg;
    bool emergency_button;
    DhtStatus_t dhtStatus;
    LoadStatus_t loadStatus;
    SystemStatus_t systemStatus;
} TelemetryData;
```

## Sensor Handling

### DHT11

The firmware periodically reads temperature and humidity. If the read succeeds, `temperature`, `humidity`, and `DHT_OK` are stored in telemetry. If the read fails, `DHT_ERROR` is reported.

### Load Input

The load input is read through ADC and converted to a percentage value. The load value is used both for telemetry and runtime threshold checks.

### ADXL345 Vibration

The ADXL345 is initialized over I2C inside `TelemetryTask`. The firmware reads raw X/Y/Z acceleration values, converts them to mg, applies a moving average filter, and stores the filtered values in telemetry:

```text
vibrationX_mg
vibrationY_mg
vibrationZ_mg
```

The debug UART may print raw and filtered ADXL345 values during development.

### Fan RPM

The fan tach signal is counted using a GPIO external interrupt. `TelemetryTask` samples the pulse count every 100 ms and calculates RPM.

Current implementation:

```text
fanRpm = pulses * 300
```

This assumes a 100 ms measurement window and a typical 2-pulse-per-revolution fan tach signal.

### Emergency Button

The emergency input is sampled periodically and exposed in telemetry as:

```text
emergency_button
```

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

The STM32 receives commands from the Raspberry Pi gateway on USART1.

```text
PING
GET_STATUS
START_MACHINE
STOP_MACHINE
RESET_FAULT
SET_LOAD_THRESHOLD:WARN=<value>;FAULT=<value>
```

Commands are line based. The receiver ignores `\r` and treats `\n` as end of command.

## UART Responses

Example responses:

```text
ACK:PING
ACK:START_MACHINE
ACK:STOP_MACHINE
ACK:RESET_FAULT
ACK:SET_LOAD_THRESHOLD
NACK:UNKNOWN_CMD
NACK:START_MACHINE:NOT_IDLE
NACK:RESET_FAULT:NO_ACTIVE_FAULT
NACK:SET_LOAD_THRESHOLD:INVALID_FORMAT
NACK:SET_LOAD_THRESHOLD:INVALID_RANGE
```

Example status message:

```text
STATUS:TEMP=27;HUM=62;LOAD=28;VIB_X=374;VIB_Y=-724;VIB_Z=-430;fanRPM=2100;emergency_button=0;STATE=MACHINE_STATE_IDLE;FAULT=FAULT_NONE;OPERATING_MODE=AUTO_MODE;DHT_STATUS=DHT_OK;LOAD_STATUS=LOAD_OK
```

## Command Behavior

### PING

Used by the Raspberry Pi gateway to check if the STM32 is alive.

Response:

```text
ACK:PING
```

### GET_STATUS

Returns the latest telemetry and machine status.

Response example:

```text
STATUS:TEMP=27;HUM=62;LOAD=28;VIB_X=374;VIB_Y=-724;VIB_Z=-430;fanRPM=2100;emergency_button=0;STATE=MACHINE_STATE_IDLE;FAULT=FAULT_NONE;OPERATING_MODE=AUTO_MODE;DHT_STATUS=DHT_OK;LOAD_STATUS=LOAD_OK
```

Field mapping:

```text
TEMP              Temperature in degrees Celsius
HUM               Relative humidity in percent
LOAD              Load input as percentage
VIB_X             Filtered ADXL345 X-axis acceleration in mg
VIB_Y             Filtered ADXL345 Y-axis acceleration in mg
VIB_Z             Filtered ADXL345 Z-axis acceleration in mg
fanRPM            Fan speed in RPM
emergency_button  Emergency input state, 0 or 1
STATE             Current machine state
FAULT             Current fault code
OPERATING_MODE    Current operating mode, currently AUTO_MODE
DHT_STATUS        DHT sensor status
LOAD_STATUS       Load sensor status
```

### START_MACHINE

Starts the machine if the system is safe.

The command is rejected if:

* The machine is already running
* A fault is active
* A sensor error exists
* Load or temperature is above the configured fault threshold

### STOP_MACHINE

Stops the machine when it is not in fault state.

If the machine is in fault state, the fault remains latched and must be cleared using `RESET_FAULT`.

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

USART usage:

```text
USART1  Protocol UART connected to Raspberry Pi gateway
USART2  Debug UART through ST-Link virtual COM port
```

## Integration Test

After flashing the STM32 and booting the Raspberry Pi gateway, the expected chain is:

```text
STM32 firmware running
Raspberry Pi edge-gateway sends PING
STM32 replies ACK:PING
Raspberry Pi sends GET_STATUS
STM32 replies STATUS:...
edge-gateway broadcasts JSON over IPC
ros2-stm32-bridge publishes /machine/telemetry
```

On the Raspberry Pi:

```sh
tail -f /var/log/edge-gateway.log
```

Expected log pattern:

```text
PI sent PING
STM32 REPLIES : ACK:PING
PI Sent : GET_STATUS
STM32 REPLIES : STATUS:TEMP=...
```

Then check ROS2 telemetry on the Raspberry Pi or host:

```sh
export ROS_DOMAIN_ID=7
ros2 topic echo /machine/telemetry
```

Expected ROS2 fields include:

```text
temperature
humidity
load
fan_rpm
vibration_x_mg
vibration_y_mg
vibration_z_mg
emergency_button
state
fault
operating_mode
dht_status
load_status
```

## Current Status

Implemented and tested:

* UART handshake with Raspberry Pi using `PING`
* Periodic `GET_STATUS` telemetry response
* DHT11 temperature/humidity telemetry
* Load percentage telemetry
* ADXL345 vibration telemetry on X/Y/Z axes
* Moving average filtering for vibration values
* Fan RPM field in telemetry and protocol
* Emergency button field in telemetry and protocol
* Machine state and fault reporting
* End-to-end publishing through the Raspberry Pi gateway to ROS2 `/machine/telemetry`

Planned next steps:

* Add a single vibration level or vibration alert field
* Add vibration warning/fault thresholds
* Improve RPM/fan fault handling
* Use emergency input as a real state/fault condition
* Clean remaining debug prints before a polished demo

## Skills Demonstrated

* STM32 firmware development
* Embedded C
* FreeRTOS task design
* UART communication
* Command-response protocol design
* Sensor integration
* I2C sensor driver integration
* ADC telemetry
* GPIO external interrupts
* PWM fan control and RPM feedback
* Moving average filtering
* Fault handling
* Machine state management
* GPIO LED control
* Integration with Embedded Linux gateway
* End-to-end STM32 to ROS2 telemetry
