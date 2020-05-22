# Eclipse IDE

Go to https://www.eclipse.org/downloads/packages/
Download the package "Eclipse IDE for C/C++ Developers" package.
Install Eclipse
Go to Help > Arduino Downloads Manager > Platforms and add Arduino AVR platform
Go to Help > Arduino Downloads Manager > Libraries and add AltSoftSerial, CAN-BUS Shield, ModbusMaster

# Floating point support for printf

This is only needed for printing debug messages and is not required for the TinyBMS to Sunny Island communication.

Edit the compiler flags in `~/.arduinocdt/packages/arduino/hardware/avr/1.8.2/platform.txt`, line 43.

```
compiler.c.elf.extra_flags=-Wl,-u,vfprintf -lprintf_flt
```

Alternatively, these flags can be set for a specific board in `boards.txt`
