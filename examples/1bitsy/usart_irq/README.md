# README

This example program echoes data sent in on USART1, 2, 3, 4, 5 and 6, using
interrupts to handle the data.

The sending of data is nonblocking.

## Board connections

| Port   | AF | Function      | Description                       |
| ------ | -- | ------------- | --------------------------------- |
| `PB6`  | 7  | `(USART1_TX)` | TTL serial output `(38400,8,N,1)` |
| `PB7`  | 7  | `(USART1_RX)` | TTL serial input `(38400,8,N,1)`  |
| `PA2`  | 7  | `(USART2_TX)` | TTL serial output `(38400,8,N,1)` |
| `PA3`  | 7  | `(USART2_RX)` | TTL serial input `(38400,8,N,1)`  |
| `PB10` | 7  | `(USART3_TX)` | TTL serial output `(38400,8,N,1)` |
| `PB11` | 7  | `(USART3_RX)` | TTL serial input `(38400,8,N,1)`  |
| `PC10` | 8  | `(UART4_TX)`  | TTL serial output `(38400,8,N,1)` |
| `PC11` | 8  | `(UART4_RX)`  | TTL serial input `(38400,8,N,1)`  |
| `PC12` | 8  | `(UART5_TX)`  | TTL serial output `(38400,8,N,1)` |
| `PD2`  | 8  | `(UART5_RX)`  | TTL serial input `(38400,8,N,1)`  |
| `PC6`  | 8  | `(USART6_TX)` | TTL serial output `(38400,8,N,1)` |
| `PC7`  | 8  | `(USART6_RX)` | TTL serial input `(38400,8,N,1)`  |
