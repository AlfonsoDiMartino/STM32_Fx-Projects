# STM32Fx Projects

This repository represent a collection of several project developed during the Embedded System exam. It is shared with my colleague and the original commits are available on his repository: https://github.com/piliguori/STM32Fx.

The most interessant projects are:

1) USB: Realization of a usb hid mouse and usb custom hid keyboard using two STM32F4 discovery boards. Both devices use the on-board accelerometer to capture the values of angular acceleration components along the three axes. In the mouse, these components determine the slider movement. Pressing the push button is associated to left click. In the keyboard, the angular acceleration components are associated with the directional arrows (up, down, left, right). Pressing the push button is associated with the space bar. Here an pratical use: https://www.youtube.com/watch?v=kPIZpKYVQI4

2) SerialBus: Realization of three mini project with three different serial interface:
  a)2C: Implementation of two Master Devices (STM32F4) that increment a count, displayed through the LEDs, and send the count value to a Slave device (STM32F3) when the push button of the board is pressed. The slave receives these values and performs the sum. 
  b) SPI: Implementation of a master device (Nucelo F4) that requests a temperature measurement when the blue button is pressed (user button) and receives the measurement from a slave device (STM32F4). The received value is displayed on an external LCD display. 
  c) UART: Comunication between a STM32F3 and a STM32F3 using UART. STM32F3 receives from a script matlab the number of samples, then it forwards this value to a STM32F4 that performs ADC e returns the sampled values to STM32F3. These values are then showed in a plot using the matlab script.
  
3) FreeRTOS: Four periodic tasks update the count digits by accessing these mutually exclusive values (using mutex), realizing a clock. A fifth periodic task reads the updated values from the other four tasks (always with the use of mutex) each time the push button is pressed. This task assumes the role of Polling Server. Every time the server reads the counting digits, it sends the read value to an external lcd device and makes the difference between a start and end time, realizing the function of a stopwatch.

