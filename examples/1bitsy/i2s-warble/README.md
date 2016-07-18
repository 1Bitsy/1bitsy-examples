# I2S Warble Demo

# Wiring

 - LED
   + LED:     Pin PA8, PA8, AF1, T4C1
   + LED:     Pin 16,  PB0, AF2, T3C3
 - serial (RX not needed)
   + TX:      Pin  2, PC10, AF8, UART4\_TX
   + RX:      Pin  3, PC11, AF8, UART4\_RX
 - button
   + GPIO:    Pin  9, PB8,  AF0, PB8
 - knob
   + ADC:     Pin 12, PC0,  ADC/DAC, ADC10
 - audio adapter
   + volume
     * ADC:   Pin 13, PC1,  ADC/DAC, ADC11
   + I2C
     * SCL:   Pin  7, PB6,  AF4, I2C1\_SCL
     * SDA:   Pin  8, PB7,  AF4, I2C1\_SDA
   + I2S
     * LRCLK: Pin 21, PB12, AF5, I2S2\_WS
     * BCLK:  Pin 22, PB3,  AF5, I2S2\_CK
     * TX:    Pin 24, PB15, AF5, I2S2\_SD
     * MCLK:  Pin 25, PC6,  AF5, I2S2\_MCLK
     * No RX

# Plan of Attack

Implement:

 - Build on JIGMOD.
   + 1BitSy
   + audio adapter
   + knob + button board
 - millisecond timer
 - serial out
 - button bounce
 - analog read
 - I2C
 - I2C to SGTL5000
 - I2S (the big one)
