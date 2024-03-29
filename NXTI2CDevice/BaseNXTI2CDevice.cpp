// NXTI2CDevice.cpp
//
// This is a base class for NXT sensors that use the I2C protocol.
//
// 2010-05-31 - Initial version, by Clinton Blackmore
//
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "BaseNXTI2CDevice.h"
#include <Wire.h>


extern "C" {
#include "../Wire/utility/twi.h"
}

// Max I2C message length is 16 bytes.  
const int BUFFER_LEN = 16;	


// Initialize static variables
uint8_t* NXTI2CDevice::_buffer = 0;


NXTI2CDevice::NXTI2CDevice(uint8_t i2c_address)
{
	// As I understand it, an I2C bus can address 127 different devices (a 7-bit quantity).
	// When used, the 7-bit quantity is shifted right one bit, and the last bit is clear
	// for a read operation and set for a write operation.  Arduino's Wire library expects
	// a 7-bit address, but most tech specs list the 8-bit address.  Hence, we drop
	// the least significant bit (and Wire.h shifts the address and sets the read/write
	// bit as appropriate.)
	_device_address = i2c_address >> 1;
	Wire.begin();

	_buffer = (uint8_t*) calloc(BUFFER_LEN, sizeof(uint8_t));
}

// READING FUNCTIONS

// Reads registers of an I2C device.
// See the documentation for your device to know what a given register
// or register range indicates.
uint8_t* NXTI2CDevice::readRegisters(
	uint8_t  start_register, 	// start of the register range
	uint8_t  bytes_to_read, 	// number of bytes to read
	uint8_t* buffer,			// (optional) user-supplied buffer
	uint8_t  buffer_length,		// (optional) length of user-supplied buffer
	bool     clear_buffer)		// should we zero out the buffer first? (optional)
{
	if (!buffer)
	{
		buffer = _buffer;
	}

	if (!buffer_length)
	{
		buffer_length = BUFFER_LEN;
	}

	bytes_to_read = min(bytes_to_read, buffer_length);	// avoid buffer overflow

	if (clear_buffer)
	{
		memset(buffer, 0, buffer_length);
	}

	// We write to the I2C device to tell it where we want to read from
	Wire.beginTransmission(_device_address);
  	Wire.write(start_register);		//Wire.send(start_register);
  	//Wire.send(bytes_to_read);
  	Wire.endTransmission();

  	// Now we can read the data from the device
	Wire.requestFrom(_device_address, bytes_to_read);

    for (uint8_t index = 0; Wire.available(); ++index)
    {
    	buffer[index] = Wire.read(); //Wire.receive();
    }

    Wire.endTransmission();

	return buffer;
}

// Reads a byte from the given register on the I2C device.
uint8_t NXTI2CDevice::readByte(uint8_t location)
{
	readRegisters(location, 1);
	return _buffer[0];
}

// Reads two bytes from the given register pair on the I2C device.
uint16_t NXTI2CDevice::readInteger(uint8_t location)
{
	readRegisters(location, 2);

	// I believe the data has the least significant byte first
	return readIntFromBuffer(_buffer);
}

// Reads four bytes from the given registers, starting at the specified location, on the I2C device.
uint32_t NXTI2CDevice::readLong(uint8_t location)
{
	readRegisters(location, 4);
	return readLongFromBuffer(_buffer);
}

// Reads a string.  Be certain that your buffer is large enough
// to hold the string and a trailing 'nul'!
char* NXTI2CDevice::readString(
		uint8_t  location, 			// starting location of the string
		uint8_t  bytes_to_read, 	// number of bytes to read
		uint8_t* buffer,			// optional user-supplied buffer
		uint8_t  buffer_length)		// length of user-supplied buffer)
{
	return (char *)readRegisters(location, bytes_to_read, buffer, buffer_length, true);
}


// WRITING FUNCTIONS

// Returns true if the write was successful.
// If not true, you may check the result by calling getWriteErrorCode.
bool NXTI2CDevice::writeRegisters(
	uint8_t  start_register, 	// start of the register range
	uint8_t  bytes_to_write, 	// number of bytes to write
	uint8_t* buffer)		// optional user-supplied buffer
{
	if (!buffer)
	{
		buffer = _buffer;
	}

	// We write to the I2C device to tell it where we want to read from and how many bytes
	Wire.beginTransmission(_device_address);
  	Wire.write(start_register);			//Wire.send(start_register);
  	//Wire.send(bytes_to_write);	// This is a guess
  	
  	// Send the data
  	for (uint8_t index = 0; index < bytes_to_write; ++index)
  	{
  		Wire.write(buffer[index]);			//Wire.send(buffer[index]);
  	}

  	_write_error_code = Wire.endTransmission();

  	return _write_error_code == 0;	// 0 indicates success
}

// Writes a byte to a given register of the I2C device
bool NXTI2CDevice::writeByte(uint8_t location, uint8_t data)
{
	return writeRegisters(location, 1, &data);
}

// Writes two bytes to a given register of the I2C device
bool NXTI2CDevice::writeInteger(uint8_t location, uint16_t data)
{
	writeIntToBuffer(_buffer, data);
	return writeRegisters(location, 2, _buffer);
}

// Writes four bytes to a given register of the I2C device
bool NXTI2CDevice::writeLong(uint8_t location, uint32_t data)
{
	writeLongToBuffer(_buffer, data);
	return writeRegisters(location, 4, _buffer);
}

// This is the status value returned from the last write command.
// A return value of zero indicates success.
// Non-zero results indicate failures.  From libraries/Wire/utility/twi.c, they are:
//          1 .. length to long for buffer
//          2 .. address send, NACK received
//          3 .. data send, NACK received
//          4 .. other twi error (lost bus arbitration, bus error, ..)
uint8_t NXTI2CDevice::getWriteErrorCode()
{
	return _write_error_code;
}

// READ SOME INFORMATION OFF OF THE DEVICE
// returns a string with the current firmware version of the device
char* NXTI2CDevice::getFirmwareVersion()
{
	return readString(0, 8);
}

// returns a string indicating the vendor of the device
char* NXTI2CDevice::getVendorID()
{
	return readString(0x08, 8);
}

// returns a string indicating the device's ID
char* NXTI2CDevice::getDeviceID()
{
	return readString(0x10, 8);
}

// It is very unusual to do this
void NXTI2CDevice::setWriteErrorCode(uint8_t code)
{
	_write_error_code = code;
}


/*
 * checkAddress()
 * this function checks to see if there is 
 * any device at a given address 
 * - deepak
 */

bool NXTI2CDevice::checkAddress()
{
	uint8_t *txBuffer;
	int8_t x = 1;
	x = twi_writeTo(_device_address, txBuffer, 0, 1) == 0;
	return (x != 0);
}


/*
 * setAddress(address)
 * this function set's the i2c address
 * for this instance to given address 
 * Note that, generally i2c address of a physical device does not change.
 * Use this function if there are multiple devices on your bus and you want to 
 * conserve processor memory from instantiating another class instance.
 * - deepak
 */

bool NXTI2CDevice::setAddress(uint8_t i2c_address)
{
	_device_address = i2c_address >> 1;
	return true;
}

uint8_t NXTI2CDevice::getAddress()
{
	return _device_address;
}
