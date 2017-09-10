# README

This example demonstrates how to use the SPI peripheral to communicate
with a ST7735R based 128x160 color TFT display.

Adafruit makes a breakout board with a 1.8" SPI TFT display and ST77535R driver
which is availalbe at <https://www.adafruit.com/product/358>. Other breakout boards
are also available online with names like HY-1.8 SPI TFT.

The ST7735R datasheet is available from <https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf>

## Board connections

The breakout board pinouts between the Adafruit and other online boards are different
so both are called out below along with the actual ST773TR pin name.

| 1Bitsy | AF | Function     | Adafruit | HY-1.8 | ST7735R | Description         |
| ------ | -- | ------------ | -------- | ------ | ------- | ------------------- |
| PB10   | 5  | SPI2 SCK     | SCK      | SCK    | SCL     | Clock               |
| PB15   | 5  | SPI2 MOSI    | MOSI     | SDA    | SDA     | Master Out Slave In |
| PB12   | 5  | SPI2 NSS     | TFT_CS   | CS     | CSX     | Slave/Chip Select   |
| PB11   |    | PortB GPIO11 | RESET    | RESET  | RESX    | TFT Hardware reset  |
| PB1    |    | PortB GPIO1  | D/C      | A0     | DC/X    | TFT Data/Command    |

In addition to the SPI interface the Data/Command pin is used to indicate whether the the serial byte
being sent to the TFT is a command (low) or data (high).
