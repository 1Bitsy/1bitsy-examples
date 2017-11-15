# README

This example program sends repeating sequence of characters "0123456789" using
semihosting. This means that if you have the 1Bitsy connected using the Black
Magic Probe to GDB on the host, all text output will land in the GDB session.
This example will also work when there is no debugger connected. In that case a
special handler will be called that sends the text via USART3 to the host
instead of the debugger session.

The sending is blocking.

## Board connections

| Port   | AF | Function      | Description                       |
| ------ | -- | ------------- | --------------------------------- |
| `PB10` | 7  | `(USART3_TX)` | TTL serial output `(38400,8,N,1)` |
