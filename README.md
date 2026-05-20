# DWIN-LCD-UART

## DWIN LCD Control

This code controls a DWIN DGUS LCD module using UART communication from an ESP32.

The LCD page is changed by sending DWIN page change commands through `Serial2`.  
Each page has different image and button behavior.

### Main Features

- Sends page change commands to the DWIN LCD
- Initializes image states when the page changes
- Controls image ON/OFF states using predefined command arrays
- Handles physical button inputs
- Supports a cleaning sequence with a 15-second timer
- Supports standby mode after cleaning is finished
- Uses two selectable image modes on page 0x01
- Moves image focus on page 0x02 and page 0x03
- Enters admin page 0x06 when POWER and STOP buttons are pressed together for 2 seconds

### Page Behavior

- Page 0x01  
  Cleaning mode starts automatically.  
  After 15 seconds or when the WASH button is pressed, cleaning ends and standby mode is enabled.

- Page 0x02  
  Image mode A is displayed.  
  The left and right buttons move the selected image focus.

- Page 0x03  
  Image mode B is displayed.  
  The left and right buttons move the selected image focus.

- Page 0x06  
  Admin page.

### Communication

The ESP32 communicates with the DWIN LCD through UART.

```c
Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
