# README

This example demonstrates the use of timers and pwm on on LED on the eval
board.

It's intended for the 1BitSquared 1BitSy STM32F415 eval board. 

This program will light one LED up on the eval board. It trigger pwm by the
initial value set up in the timers output compare register. For this example we
use a large value to make the led bright. The duty cycle is defined by the oc
value / timer period.

Additionally to the PA8 TIM1 OC1 pin that is connected to the LED we also
configure PB6 TIM4 OC1 with similar settings. The TIM4 runs at half of the TIM1
frequency that can be seen if you connect an oscilloscope.

This demo shows the difference in setting up an advanced timer and a basic timer.

## Board connections

*none required*
