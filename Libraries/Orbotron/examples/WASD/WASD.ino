#include <orbotron_device.h>
#include <orbotron_buffer.h>
#include <orbotron_translator.h>
#include <hid_keys.h>
#include <chart_4.h>

//change value below to SpaceOrb360, SpaceBall4000, or SpaceBall5000
Logical_orbotron orb_buffer( SpaceOrb360 );
Orbotron_translator translator;
Orbotron_device orb_device;

// Some people don't like the orb tilting (yaw), so consider
// commenting out the second binding.  If you do, change the "2" to "1"
// in set_axis_mouse_bindings below.  If you want to reverse an axis, set
// the third number (scale) to a negative value.
PROGMEM Axis_mouse_binding axis_mouse_bindings[] =
  {
    //{ 5, MOUSE_AXIS_X, 4, 0, 2 },
    { 5, MOUSE_AXIS_X, 4, 288, 2 },
    { 3, MOUSE_AXIS_Y, -4, 288, -2 }
  };

// Binds the first two buttons to mouse1, mouse2.  You can bind to mouse3!
// NOTE that here the first argument is the button mask for the orb in
// LOGICAL buttons--so with default chording on the spaceorb, 0x01 is button "C"
// (here mapped to left-mouse) and 0x01 << 1 is button "D" (here mapped to right-mouse)
PROGMEM Button_mouse_binding button_mouse_bindings[] =
  {
    { 0x01, 0x01 },
    { 0x01 << 1, 0x01 << 1 }
  };

// sets buttons 4-7 to keyboard 1-4 (eg for weapon select)
PROGMEM Button_key_binding button_key_bindings[] =
  {
    { 0x01 << 2, 0, KEY_1 },
    { 0x01 << 3, 0, KEY_2 }, 
   { 0x01 << 4, 0, KEY_3 },
    { 0x01 << 5, 0, KEY_4 },
  };

// finally, demo that you can press keys with axes, we'll use the z-axis
// to crouch (CTRL) or jump (space) as in the half-life bindings.
// note that since control is a modifier it goes in a different place than
// a regular key
PROGMEM Axis_key_binding axis_key_bindings[] =
  {
    { 2, 773, 1023, MOD_CONTROL_LEFT, 0 },
    { 2, 0, 250, 0, KEY_SPACE }
  };


void setup()
{
  Serial1.begin( 9600 );

  translator.default_setup( orb_buffer.orb_type );

  // WASD_fourway_binding is built in since it will be so common;
  // it is just 
  //   const PROGMEM Fourway_keyboard_binding WASD_fourway_binding = { 0, 1, KEY_S, KEY_W, KEY_D, KEY_A, 75*75 };
  // so you can make your own easily.
  translator.set_fourway_bindings( &WASD_fourway_binding, 1 );
  translator.set_axis_mouse_bindings( axis_mouse_bindings, 2 );
  translator.set_button_mouse_bindings( button_mouse_bindings, 2 );
  translator.set_button_key_bindings( button_key_bindings, 4 );
  translator.set_axis_key_bindings( axis_key_bindings, 2 );

  // In general, it's best to set joystick reports off if you're not using
  // the orb as a joystick device; it frees up the bus for updates
  translator.send_joystick_reports = false;
}


void loop()
{
  orbotron_checkinit( orb_buffer, translator, orb_device );
  orbotron_translate( orb_buffer, translator, orb_device );
}
