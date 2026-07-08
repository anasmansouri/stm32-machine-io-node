# STM32 Machine I/O Node

STM32 firmware for a small industrial-style machine monitoring and control node.

The STM32 reads real machine inputs, controls physical outputs, manages machine state and fault logic, measures fan RPM, and communicates with a Raspberry Pi edge gateway over UART.

## Project Goal

This project demonstrates the microcontroller side of an end-to-end embedded monitoring platform.

The STM32 acts as the machine I/O node:

* Collects telemetry from sensors and inputs
* Controls fan PWM and status LEDs
* Measures fan tachometer pulses
* Handles emergency stop input
* Evaluates runtime warning/fault conditions
* Exposes machine status over a UART command-response protocol
* Integrates with a Raspberry Pi Yocto/ROS2 gateway and Qt/QML HMI

## Related Repositories

```text
Raspberry Pi / Yocto / ROS2 gateway : https://github.com/anasmansouri/machine-monitoring-edge-gateway
Qt/QML HMI                         : https://github.com/anasmansouri/machine-monitoring-hmi
```

## System Architecture

```text
+------------------------------+
| STM32 Machine I/O Node       |
|                              |
| Inputs                       |
| - DHT11 temperature/humidity |
| - Load input through ADC     |
| - Fan tachometer feedback    |
| - Emergency stop input       |
|                              |
| Outputs                      |
| - PWM fan control            |
| - Red/yellow/green LEDs      |
|                              |
| Logic                        |
| - Machine state management   |
| - Fault handling             |
| - UART protocol              |
| - Optional vibration fields  |
+---------------+--------------+
                |
                | USART1 / UART
                v
+---------------+--------------+
| Raspberry Pi Edge Gateway    |
| Yocto Linux + ROS2           |
+---------------+--------------+
                |
                | ROS2
                v
+---------------+--------------+
| Qt/QML HMI                   |
+------------------------------+
```

End-to-end telemetry flow:

```text
STM32 sensors and state
  -> TelemetryTask
  -> latestTelemetry
  -> GET_STATUS response over USART1
  -> Raspberry Pi edge-gateway
  -> Unix socket JSON
  -> ROS2 /machine/telemetry
  -> Qt/QML HMI
```

## Current Hardware

Active in the current demo:

* STM32 Nucleo board
* DHT11 / KY-015 temperature and humidity sensor
* Load input / potentiometer through ADC
* 4-pin PWM fan
* Fan tach/RPM feedback
* Emergency stop input
* Red/yellow/green status LED module
* UART connection to Raspberry Pi

Optional / temporarily disabled:

* ADXL345 vibration sensor over I2C

The vibration fields are kept in the UART protocol for compatibility with the gateway and ROS2 message. When the vibration sensor is disabled, the firmware can publish vibration values as `0`.

## Pin Mapping

| Function | STM32 pin | Notes |
|---|---:|---|
| Load input | `PA0 / ADC1_CH0` | Analog load value converted to percent |
| Fan PWM | `PA5 / TIM2_CH1` | 25 kHz PWM fan control |
| Emergency input | `PC0` | Internal pull-up, active-low input |
| Fan tach input | `PC1 / EXTI1` | Internal pull-up, falling-edge interrupt |
| Yellow LED | `PB4` | Warning state output |
| Green LED | `PB5` | Running state output |
| Red LED | `PB10` | Fault state output |
| DHT11 data | `PB6` | Single-wire DHT11 data line |
| Optional I2C SCL | `PB8 / I2C1_SCL` | ADXL345 if re-enabled |
| Optional I2C SDA | `PB9 / I2C1_SDA` | ADXL345 if re-enabled |
| Raspberry Pi UART | `USART1` | Command/response protocol |
| ST-LINK debug UART | `USART2` | Debug prints over virtual COM port |

## Main Features

* FreeRTOS-based task architecture
* UART command-response protocol on USART1
* DHT11 temperature and humidity telemetry
* Load percentage telemetry from ADC
* Fan PWM control using TIM2 Channel 1
* Fan RPM measurement from tach pulses on PC1 / EXTI1
* Emergency button state reporting
* Machine state management
* Load and temperature threshold handling
* Optional vibration telemetry and threshold handling
* Fault detection and latched fault behavior
* State-based status LED and fan control
* Integration with Raspberry Pi Yocto/ROS2 gateway

## Firmware Tasks

```text
UartRxTask
  Receives UART command bytes from Raspberry Pi on USART1 and pushes complete commands into a queue.

CommandTask
  Consumes queued commands, reads the latest telemetry snapshot, calls the protocol handler, and sends the response over USART1.

TelemetryTask
  Reads inputs, updates latestTelemetry, calculates fan RPM, reads emergency input, and evaluates runtime machine state.

DefaultTask
  Kept minimal.
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
    int32_t vibration_level_mg;
    bool emergency_button;
    DhtStatus_t dhtStatus;
    LoadStatus_t loadStatus;
    SystemStatus_t systemStatus;
} TelemetryData;
```

## Sensor and Input Handling

### DHT11

The firmware periodically reads temperature and humidity.

If the read succeeds:

```text
temperature
humidity
DHT_OK
```

If the read fails:

```text
DHT_ERROR
```

### Load Input

The load input is read through ADC and converted to a percentage value.

The load value is used for:

* Telemetry
* Warning threshold evaluation
* Fault threshold evaluation

### Fan RPM

The fan tach signal is connected to `PC1` and counted using `EXTI1` falling-edge interrupts.

The current setup uses the STM32 internal pull-up on `PC1`, so no external pull-up resistor is required for the fan tach line.

RPM calculation:

```text
rpm = (pulses * 60000) / (2 * elapsedMs)
```

This assumes a typical 2-pulse-per-revolution PC fan tach signal.

### Emergency Stop

The emergency input is sampled periodically and exposed in telemetry as:

```text
emergency_button
```

The emergency stop input is active-low with internal pull-up enabled.

If the emergency input is active while starting, resetting, or running, the firmware enters a latched fault state:

```text
FAULT_EMERGENCY_STOP
```

### Optional ADXL345 Vibration Sensor

The ADXL345 vibration feature is currently optional.

Recommended compile-time switch:

```c
#define ENABLE_VIBRATION_SENSOR 0
```

Behavior:

```text
ENABLE_VIBRATION_SENSOR = 0
  ADXL345 is not required.
  VIB_X, VIB_Y, VIB_Z and VIB_LEVEL are reported as 0.
  Gateway, ROS2 and HMI compatibility is preserved.

ENABLE_VIBRATION_SENSOR = 1
  ADXL345 is initialized over I2C.
  X/Y/Z acceleration is read and filtered.
  VIB_LEVEL is calculated and can be used by the state machine.
```

If the sensor is re-enabled, the expected wiring is:

```text
ADXL345 VCC -> 3.3V
ADXL345 GND -> GND
ADXL345 SCL -> PB8 / I2C1_SCL
ADXL345 SDA -> PB9 / I2C1_SDA
ADXL345 CS  -> 3.3V
ADXL345 SDO -> GND
```

For reliable I2C operation, use pull-up resistors to 3.3V on SDA and SCL if the module does not already provide them.

## Fan PWM Control

The fan PWM signal is generated by `TIM2_CH1`.

Current PWM timer configuration:

```text
TIM2 prescaler: 0
TIM2 period / ARR: 3359
PWM frequency: ~25 kHz
Initial pulse: 1680
```

The fan control logic is abstracted in `fan_control.c/.h`:

```c
Fan_Init(&htim2, TIM_CHANNEL_1);
Fan_SetDutyPercent(...);
Fan_Stop();
```

The machine state layer does not access the timer directly. It calls the fan control API from `Machine_ApplyStateOutputs()`.

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
FAULT_LOAD_TOO_HIGH
FAULT_OVERTEMPERATURE
FAULT_DHT_SENSOR_ERROR
FAULT_LOAD_SENSOR_ERROR
FAULT_EMERGENCY_STOP
FAULT_VIBRATION_HIGH
```

`FAULT_VIBRATION_HIGH` is kept for the optional vibration feature. In the current hardware setup, vibration values can be kept at `0`, so this fault will not be triggered by an absent sensor.

## State-based Outputs

| Machine state | LED output | Fan output |
|---|---|---|
| `MACHINE_STATE_IDLE` | All LEDs off | Fan off |
| `MACHINE_STATE_RUNNING` | Green LED | `FAN_DUTY_IN_RUNNING_STATE` |
| `MACHINE_STATE_WARNING` | Yellow LED | `FAN_DUTY_IN_WARNING_STATE` |
| `MACHINE_STATE_FAULT` | Red LED | Fan off |

Current fan duty constants:

```text
FAN_DUTY_IN_RUNNING_STATE = 40
FAN_DUTY_IN_WARNING_STATE = 80
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
SET_VIBRATION_THRESHOLD:WARN=<value>;FAULT=<value>
```

`SET_VIBRATION_THRESHOLD` is retained for future use when the vibration sensor is re-enabled.

Commands are line based. The receiver ignores `\r` and treats `\n` as the end of a command.

## UART Responses

Example responses:

```text
ACK:PING
ACK:START_MACHINE
ACK:STOP_MACHINE
ACK:RESET_FAULT
ACK:SET_LOAD_THRESHOLD
ACK:SET_VIBRATION_THRESHOLD
NACK:UNKNOWN_CMD
NACK:START_MACHINE:NOT_IDLE
NACK:START_MACHINE:FAULT_EMERGENCY_STOP
NACK:START_MACHINE:FAULT_DHT_SENSOR_ERROR
NACK:START_MACHINE:FAULT_LOAD_SENSOR_ERROR
NACK:START_MACHINE:FAULT_LOAD_TOO_HIGH
NACK:START_MACHINE:FAULT_OVERTEMPERATURE
NACK:START_MACHINE:FAULT_VIBRATION_HIGH
NACK:RESET_FAULT:NO_ACTIVE_FAULT
NACK:RESET_FAULT:FAULT_EMERGENCY_STOP
NACK:RESET_FAULT:FAULT_DHT_SENSOR_ERROR
NACK:RESET_FAULT:FAULT_LOAD_SENSOR_ERROR
NACK:RESET_FAULT:FAULT_LOAD_TOO_HIGH
NACK:RESET_FAULT:FAULT_OVERTEMPERATURE
NACK:RESET_FAULT:FAULT_VIBRATION_HIGH
NACK:SET_LOAD_THRESHOLD:INVALID_FORMAT
NACK:SET_LOAD_THRESHOLD:INVALID_RANGE
NACK:SET_VIBRATION_THRESHOLD:INVALID_FORMAT
NACK:SET_VIBRATION_THRESHOLD:INVALID_RANGE
```

Example `GET_STATUS` response with vibration disabled:

```text
STATUS:TEMP=27;HUM=62;LOAD=28;VIB_X=0;VIB_Y=0;VIB_Z=0;VIB_LEVEL=0;fanRPM=1200;emergency_button=0;STATE=MACHINE_STATE_IDLE;FAULT=FAULT_NONE;OPERATING_MODE=AUTO_MODE;DHT_STATUS=DHT_OK;LOAD_STATUS=LOAD_OK
```

Field mapping:

```text
TEMP              Temperature in degrees Celsius
HUM               Relative humidity in percent
LOAD              Load input as percentage
VIB_X             Optional vibration X value in mg, 0 when disabled
VIB_Y             Optional vibration Y value in mg, 0 when disabled
VIB_Z             Optional vibration Z value in mg, 0 when disabled
VIB_LEVEL         Optional vibration level in mg, 0 when disabled
fanRPM            Fan speed in RPM
emergency_button  Emergency input state, 0 or 1
STATE             Current machine state
FAULT             Current fault code
OPERATING_MODE    Current operating mode, currently AUTO_MODE
DHT_STATUS        DHT sensor status
LOAD_STATUS       Load sensor status
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

### START_MACHINE

Starts the machine if the system is safe.

The command is rejected if:

* The machine is already in fault state
* The machine is not idle
* Emergency stop is active
* DHT sensor error exists
* Load sensor error exists
* Load is above the configured fault threshold
* Temperature is above the configured fault threshold
* Vibration level is above the configured fault threshold, if vibration is enabled

### STOP_MACHINE

Stops the machine when it is not in fault state.

If the machine is in fault state, the fault remains latched and must be cleared using `RESET_FAULT`.

### RESET_FAULT

Clears the fault only if current input values are safe.

The reset is rejected while an active fault condition still exists.

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

### SET_VIBRATION_THRESHOLD

Updates warning and fault thresholds for vibration.

Example:

```text
SET_VIBRATION_THRESHOLD:WARN=2500;FAULT=3000
```

This command is retained for future use when the vibration sensor is re-enabled.

## Runtime Behavior

Runtime supervision is active while the machine is in:

```text
MACHINE_STATE_RUNNING
MACHINE_STATE_WARNING
```

Fault priority during runtime evaluation:

```text
1. Emergency stop
2. DHT sensor error
3. Load sensor error
4. Load above fault threshold
5. Temperature above fault threshold
6. Vibration level above fault threshold, if enabled
7. Warning thresholds
```

If a runtime fault condition appears, the state changes to:

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
Baud rate:    115200
Data bits:    8
Parity:       None
Stop bits:    1
Flow control: None
```

## Debug UART

Debug output is sent over ST-LINK virtual COM port using USART2.

Open it on Linux with:

```bash
picocom -b 115200 /dev/ttyACM0
```

Exit `picocom`:

```text
Ctrl + A, then Ctrl + X
```

## Current Status

Working:

* FreeRTOS task structure
* UART receive queue and command task
* `PING` / `GET_STATUS` / `START_MACHINE` / `STOP_MACHINE` / `RESET_FAULT`
* Load threshold command
* Optional vibration threshold command retained
* DHT11 telemetry
* Load telemetry
* Fan PWM control
* Fan RPM feedback using internal pull-up on PC1
* Emergency stop fault handling
* State-based fan and LED outputs
* Gateway/HMI-compatible UART status format

Current hardware demo focuses on:

```text
Temperature
Humidity
Load
Fan RPM
Emergency stop
Machine state
Fault state
Status LEDs
Load threshold configuration
```

## Future Improvements

* Re-enable or replace the ADXL345 vibration sensor
* Move optional vibration code behind a clean compile-time switch
* Move fan tach logic into a dedicated module
* Move pin definitions into an `app_config.h` file
* Add a clean wiring diagram to the documentation
* Add automated unit tests for protocol parsing and state transitions
