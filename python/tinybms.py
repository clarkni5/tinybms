import numpy as np

from pymodbus.client.sync import ModbusSerialClient


# Modbus client
port = '/dev/ttyUSB0'
client = ModbusSerialClient(method='rtu', port=port, timeout=2, baudrate=115200)
client.connect()

# Make a request to wake up the BMS
client.read_holding_registers(0, 1, unit=0xAA)

#
# Basic examples
#

# Get a single register
temperature = client.read_holding_registers(48, 1, unit=0xAA).registers[0]
print(f"BMS temperature: {temperature/10}C")

# Get a range of registers
registers = client.read_holding_registers(0, 16, unit=0xAA).registers
for cell, value in enumerate(registers):
    print(f"Cell {16-cell}: {value/10000}V")

# Get a float value stored across 2 registers
registers = client.read_holding_registers(36, 2, unit=0xAA).registers
pack_voltage = np.array(registers, dtype=np.uint16).view(dtype=np.float32)[0]
print(f"Pack voltage: {pack_voltage}V")

# Get a 32-bit unsigned integer value stored across 2 registers
registers = client.read_holding_registers(46, 2, unit=0xAA).registers
soc = np.array(registers, dtype=np.uint16).view(dtype=np.uint32)[0]
print(f"State of charge: {soc/1000000}%")

#
# Get more data
#

def convert(array, type):
    return np.array(array, dtype=np.uint16).view(dtype=type)[0]

def read_registers(address, count):
    return client.read_holding_registers(address, count, unit=0xAA).registers

lifetime_counter = convert(read_registers(32, 2), np.uint32)
print(f"Lifetime counter: {lifetime_counter}s")

time_left = convert(read_registers(34, 2), np.uint32)
print(f"Estimated time left: {time_left}s")

pack_current = convert(read_registers(38, 2), np.float32)
print(f"Pack current: {pack_current}A")

max_discharge_current = read_registers(102, 1)[0]
print(f"Max discharge current: {max_discharge_current/10}A")

max_charge_current = read_registers(103, 1)[0]
print(f"Max charge current: {max_charge_current/10}A")

charge_count = read_registers(111, 1)[0]
print(f"Charging count: {charge_count}")

min_cell = read_registers(40, 1)[0]
print(f"Min cell votlage: {min_cell/1000}V")

max_cell = read_registers(41, 1)[0]
print(f"Max cell votlage: {max_cell/1000}V")

cell_diff = read_registers(104, 1)[0]
print(f"Max cell voltage difference: {cell_diff/10000}V")
