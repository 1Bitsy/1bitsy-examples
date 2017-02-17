# README

This example program sends repeating sequence of characters "0123456789" on 
USART1, USART2, USART3, UART4, UART5 and USART6

The sending is blocking.

## Board connections

| Port   | AF | Function      | Description                       |
| ------ | -- | ------------- | --------------------------------- |
| `PB6`  | 7  | `(USART1_TX)` | TTL serial output `(38400,8,N,1)` |
| `PA2`  | 7  | `(USART2_TX)` | TTL serial output `(38400,8,N,1)` |
| `PB10` | 7  | `(USART3_TX)` | TTL serial output `(38400,8,N,1)` |
| `PC10` | 8  | `(UART4_TX)`  | TTL serial output `(38400,8,N,1)` |
| `PC12` | 8  | `(UART5_TX)`  | TTL serial output `(38400,8,N,1)` |
| `PC6`  | 8  | `(USART6_TX)` | TTL serial output `(38400,8,N,1)` |
