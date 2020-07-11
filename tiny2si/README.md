# Development environment

## Eclipse IDE

Install Eclipse IDE.

0. Go to [Eclipse Packages](https://www.eclipse.org/downloads/packages/)
0. Download the **Eclipse IDE for C/C++ Developers** package.
0. Install Eclipse

### Configuration (optional)

For better performance, increase the `-Xms` value to match the `-Xmx` value in the `/opt/eclipse/eclipse.ini` file.

```
-Xms1024m
-Xmx1024m
```

Removing the Mylyn integrations may improve perforamnce.

0. Go to **Help** > **About Eclipse IDE**
0. Click on **Installation details**
0. Select all **Mylyn** plugins (except Mylyn WikiText)
0. Click on **Uninstall**
0. Click on **Finish**

### Git integration (optional)

Install the Git integration.

0. Go to **Help** > **Install New Software**
0. Work with: Select **Latest Eclipse Release**
0. Select **Collaboration** > **Git integration for Eclipse**
0. You will be prompted to restart Elcipse
0. Go to **Window** > **Perspective** > **Customize Perspective**
0. Click on the **Action Set Availability** tab
0. Select **Git**
0. Click on **Finish**

Configure quick diff to show changes in the editor.

0. Go to **Window** > **Preferences**
0. Go to **General** > **Editors** > **Text Editors** > **Quick Diff**
0. Use this reference source: **A Git Revision**

### Arduino integration

Add the required platform.

0. Go to **Help** > **Arduino Downloads Manager** > **Platforms**
0. Add **Arduino AVR platform**

Add the required libraries.

0. Go to **Help** > **Arduino Downloads Manager** > **Libraries**
0. Add **AltSoftSerial**, **CAN-BUS Shield**, and **ModbusMaster**

Create a launch target.

0. Select **New Launch Target...** from the **Launch Target** dropdown in the launch toolbar (the third dropdown).
0. Select ** Aruino**
0. Click on **Next**
0. Target name: **Arduino Mega**
0. Serial port: Use the default value
0. Board type: **Arduino Mega or Mega 2560**
0. Processor: **ATmega2560 (Mega 2560)**
0. Programmer: **AVR ISP**
0. Click on **Finish**

### Floating point support (optional)

This is only necessary for printing floating point values in debug messages using printf and is not required for the TinyBMS to Sunny 
Island communication.

Edit the compiler flags in `~/.arduinocdt/packages/arduino/hardware/avr/1.8.2/platform.txt`, line 43.

```
compiler.c.elf.extra_flags=-Wl,-u,vfprintf -lprintf_flt
```

Alternatively, these flags can be set for a specific board in `boards.txt`

### Import the code

0. Click on **File** > **Import...**
0. Select **General** > **Project from Folder or Archive**
0. Click on **Next**
0. Input source: Select the `tiny2si` directory
0. Click on **Finish**

### Build the code

Use the build icon (the hammer) in the launch toolbar to build the code.
Or, from the menu, go to **Project** > **Build all** (ctrl+B).

### Upload the code

Connect the Arduino to the USB port.

Update the launch target serial port if necessary.

0. Click on the edit icon (the cog) in the **Launch Target** dropdown in the launch toolbar (the third dropdown).
0. Serial Port: Select the appropriate port (e.g. /dev/ttyUSB0)
0. Click on **Apply and Close**

Use the run icon (the play button) in the launch toolbar to upload the code.
Or, from the menu, go to **Run** > **Run** (Ctrl+F11).

### Serial monitor

0. Click on the toolbar icon to **Open a terminal** (Shift+Ctrl+Alt+T).
0. Choose terminal: **Arduino Serial Monitor**
0. Board Name: **Arduino Mega**
0. Baud Rate: **9600**
0. Click on **OK**

## Serial logger

Use `screen` as a command line serial monitor. 
The `-L` option will log the output to a file named `screenlog.0`.

```
screen -L /dev/ttyUSB0 9600
```

To end the screen session, press `Ctrl+A`, type `:quit` and press `Enter`.

# Hardware

## Arduino Mega (2650 Pro Mini) connections

### Tiny BMS Modbus

Use a JST 2.0mm 4-pin connector to connect to the TinyBMS.
Modbus communication is over RS232 serial communication.
 
Connect the modbus wires from the TinyBMS to the Arduino Mega Serial1 pinout.

* **5V** (white) > **X** - not used
* **Tx** (yellow) > **pin 19** - data input pin (Serial1 RX)
* **Rx** (black) > **pin 18** - data output pin (Serial1 TX)
* **GND** (red) > **ground** - ground reference for I/O pins

### Sunny Island CAN bus

TODO: Note about can bus clock speed.

Connect the MCP2515 breakout board to the Arduino Mega using the SPI pinout.

* **INT** > **pin 21** - interrupt output pin (not used in this sketch)
* **SCK** > **pin 52** - clock input pin for SPI interface
* **SI** > **pin 51** - data input pin for SPI interface
* **SO** > **pin 50** - data output pin for SPI interface
* **CS** > **pin 53** - chip select input pin for SPI interface
* **GND** > **ground** - ground reference for logic and I/O pins
* **VCC** > **5V** - positive supply for logic and I/O pins
 
Alternatively, you can connect the MCP2515 breakout board to the Arduino Mega using the ICSP pinout.

* **INT** > ~pin 21~ - interrupt output pin (not used in this sketch)
* **SCK** > **pin 20** - clock input pin for SPI interface (middle inside pin)
* **SI** > **pin 21** - data input pin for SPI interface (middle outside pin)
* **SO** > **pin 22** - data output pin for SPI interface (inside near pin 53)
* **CS** > **pin 53** - chip select input pin for SPI interface
* **GND** > **ground** - ground reference for logic and I/O pins (outside near pin 30)
* **VCC** > **5V** - positive supply for logic and I/O pins (outside near pin 52)

### Telemetry WiFi

TODO

ESP8266

Serial3 - D15 RX, D14 TX