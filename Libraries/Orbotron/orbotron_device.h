#ifndef ORBOTRON_DEVICE_H
#define ORBOTRON_DEVICE_H

#include <arduino.h>

class Orbotron_device 
{
 public:
  bool init_string_sent;
  unsigned long millis_at_init;
  JoyState_t joy_state;

 Orbotron_device() :
  init_string_sent(false)
    {
      millis_at_init = millis();
      joy_state.xAxis = 0;
      joy_state.yAxis = 0;
      joy_state.zAxis = 0;
      joy_state.xRotAxis = 0;
      joy_state.yRotAxis = 0;
      joy_state.zRotAxis = 0;
      joy_state.buttons = 0;
    }

  void send_joystick_report( unsigned short x, 
			     unsigned short y, 
			     unsigned short z,
			     unsigned short rx, 
			     unsigned short ry, 
			     unsigned short rz, 
			     unsigned short btn ) 
  {
    joy_state.xAxis = x;
    joy_state.yAxis = y;
    joy_state.zAxis = z;
    joy_state.xRotAxis = rx;
    joy_state.yRotAxis = ry;
    joy_state.zRotAxis = rz;
    joy_state.buttons = btn;
    Joystick.setState(&joy_state);
  }

  void send_keyboard_report( unsigned char modifiers, int count, unsigned char * buttons )
  {
    Keyboard.send_keys( modifiers, count, buttons );
  }

  void send_single_keyboard_report( unsigned char keyStroke, unsigned char modifiers )
  {
    //Serial.print("Key1");
  }

  void send_mouse_report( unsigned char buttons,
			  char x,
			  char y,
			  char wheel )
  {
    Mouse.buttons(buttons);
    Mouse.move( x, y, wheel );
  }

};

#endif
