# README

DAC test with DMA and timer 2 trigger and real time sample generation.

Timer 2 is setup to provide a trigger signal on OC1

The DAC is setup on channel 1 and 2 to output a sample on the timer trigger.

DMA controller 1, stream 5, channel 7 is used to move data from a
predefined array in circular mode when the DAC1 requests, and DMA1, stream 6,
channel 7 is used to move the same data to DAC2.

DMA half transfer complete occurs when half of the data array has been passed
through. and DMA transfer complete occures when the second half of the data has
been transferred. We are filling in new wave data into the array every half of
the array length while the other half is being transferred to the DAC outputs.

PA1 pin is high when the MCU is busy generating samples, this indicates how
fast we can generate the waveform.

The analog output appears on PA4 (DAC channel 1) and PA5 (DAC channel 2).

We also change the frequency of the output signal every 100 buffer transfer
cycles.

