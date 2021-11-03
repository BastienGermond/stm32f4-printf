# STM32F4 - Printf

This is a short example commented on how to use printf during stm32f4
development through UART.

## How to use ?

This project use a STM32F401RE but should work on any STM32F4xx.

Flash this program using platformio:

```
pio run -t upload
```

Then you will be able to use the UART from the flashing cable with platformio:

```
pio device monitor -b 112500
```
