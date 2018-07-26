# STM32Fx

This repository represent a collection of several project developed during the Embedded System exam. It is shared with my colleague and the original commits are available on his repository. Refer to it : https://github.com/piliguori/STM32Fx.

The most interessant projects are:

1) USB: Realization of a usb hid mouse and usb custom hid keyboard using two STM32F4 discovery boards. Both devices use the on-board accelerometer to capture the values of angular acceleration components along the three axes. In the mouse, these components determine the slider movement. Pressing the push button is associated to left click. In the keyboard, the angular acceleration components are associated with the directional arrows (up, down, left, right). Pressing the push button is associated with the space bar. Here an pratical use: https://www.youtube.com/watch?v=kPIZpKYVQI4

