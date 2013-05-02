#include <ctype.h>

byte SWval;
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("This sketch simply takes input from the OrbShield's serial port");
  Serial.println("and sends it out the serial port.  DIP switches should be set with");
  Serial.println("switch 1 (nearest to the serial port) ON, switch 2 OFF");
  Serial.println("Serial dump follows:\n");
}

const char spaceball_setup_string[] ="\rCB\rNT\rFT?\rFR?\rP@r@r\rMSSV\rZ\rBcCcCcC\r";
const char magellan_setup_string[] = "\rm3\rpBB\rz\r";

byte has_sent=0;

void loop()
{
  delay(500);

  Serial.print(".");

  while ( Serial1.available() > 0 )
  {
     {
      SWval = Serial1.read();
      Serial.print( SWval );
      Serial.print( "(" );
      Serial.print( SWval, DEC );
      Serial.print( ")\n" );     
     }
  }
}

