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



// Joshua Vasquez
// Wireless NXT Tread Control 
//   via Xbee Shield
// Date last revised: July 2, 2012


/* The following Program....
   1. sets up the NXT Motors
   2. Listens for a valid serial data packet.
   3. interprets the data sent over the xBee shields.
   4. converts serial data to valid motor speeds
   5. writes motor speeds to motors!
*/


#include <NXTI2CDevice.h>
#include <NXTMMX.h>
#include <Wire.h>

const int loadTime = 1000; // delay time in void setup.
const char PacketStart = '~';  // the start of the data packet

int leftSpeed = 0;   // the starting speeds of the left
int rightSpeed = 0;  //     and right motors.

// Instantiate an NXT controller called N:
NXTMMX  N; 


void setup()
{ 
  // Open a Serial channel between Xbees:
  Serial.begin(9600); 
  
  // manditory reset:
  N.reset();   
  /* 
       Delay for loadTime to ensure that both 
       the Serial communication is ready and that 
       the NXT has had enough time to reset.
  */
  delay(loadTime);
  Serial.println("I'm ready for action!");
}

void loop()
{
 while(Serial.available() < 1);  // do nothing until a byte arrives.
 if (Serial.read() == PacketStart)
 {
   // wait for 2 more bytes:
   while (Serial.available() < 2);
   
   // Read the next two bytes as left and right speeds:
   leftSpeed = Serial.read();
   rightSpeed = Serial.read();
   
   // Update NXT Motor Speeds:
   
   /* NXT library only accepts unsigned speeds.
    Convert input range of -100 to 100 to a format that
    the runUnlimited() method will accept:
   */
   
   // Convert leftSpeed and write it:
   if (leftSpeed >= 0)
   {
     N.runUnlimited(  MMX_Motor_2,
                      MMX_Direction_Forward,
                      leftSpeed);
   }
   else
   {
     N.runUnlimited(  MMX_Motor_2,
                      MMX_Direction_Reverse,
                      (-leftSpeed) );
   }
   
   // Convert rightSpeed and write it:
 if (rightSpeed >= 0)
   {
     N.runUnlimited(  MMX_Motor_1,
                      MMX_Direction_Forward,
                      rightSpeed);
   }
   else
   {
     N.runUnlimited(  MMX_Motor_1,
                      MMX_Direction_Reverse,
                      (-rightSpeed) );
   }    
 }
}
 
