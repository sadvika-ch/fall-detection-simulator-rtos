# Fall Detection Simulator (RTOS)

## Overview

This project implements a **Fall Detection System using STM32 and MPU6050 accelerometer sensor**.  
The system detects sudden impacts followed by abnormal tilt angles to identify a potential fall event.

The design uses **FreeRTOS multitasking**, sensor data processing, and **task notifications** to simulate a real-time fall detection system.


## Hardware Components

- STM32 Microcontroller
- MPU6050 Accelerometer + Gyroscope
- UART Serial Interface
- LEDs for alert indication


## Software Tools

- STM32CubeIDE
- FreeRTOS
- Embedded C
- HAL Drivers


## System Architecture

The system uses multiple RTOS tasks:

### Sensor Task
Reads accelerometer and gyroscope data from MPU6050 using I2C.

### Fall Detection Task
Calculates resultant acceleration and tilt angles to detect fall conditions.

### Alert Task
Triggered when a fall is detected and prints alert message via UART.



## Detection Logic

1. MPU6050 sensor continuously reads acceleration data.
2. Resultant acceleration magnitude is calculated.
3. If acceleration exceeds threshold (impact detected), tilt angle is calculated.
4. If tilt angle exceeds predefined limit, a **fall event is triggered**.


## RTOS Concepts Used

- Multitasking
- Software Timers
- Task Notifications
- Inter-task Communication
- Real-time scheduling


## Alert Output

If fall is detected:
FALL DETECTED

LED indicators toggle and UART prints alert.

## Learning Outcomes

- Sensor interfacing using I2C
- Real-time data processing
- RTOS task synchronization
- Embedded system event detection

