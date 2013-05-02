#include <orbotron_device.h>
#include <orbotron_buffer.h>
#include <orbotron_translator.h>
#include <hid_keys.h>
#include <chart_4.h>

/*
This demonstrates the variable gain features of the Orbotron 9000,
showing that all axes (or individual axes) can be set to have different
gain or sensitivity.

The difference between gain and sensitivity is subtle.  Sensitivity refers
to a "curve" which in its default setting is flattish in the middle and 
slopes quickly away--making the orb "feel" more delicate in the middle of its
range but giving the ability to still make large motions near the edge.

Gain, on the other hand, acts as a multiplier to the orb's position.  A high
gain, for example, pushes the orb further along its sensitivity curve
for a given displacement.  Negative values for gain make the orb more "mushy".

"Precision Gain" is a feature which changes the orb's sensitivity when a button
or buttons are pressed.  This lets you have rapid response when you need it
but get more fine-grained control at the push of a button.

This sketch is meant as a demonstration only.  Upload it to your Orbotron,
then go to the game controllers applet in windows and check Properties;
try displacing the orb a bit in x/y and pressing the "A" button; notice that
the positions move (X becomes more sensitive, but other axes become less 
sensitive).
*/

//change value below to SpaceOrb360, SpaceBall4000, or SpaceBall5000

Logical_orbotron orb_buffer( SpaceOrb360 );
Orbotron_translator translator;
Orbotron_device orb_device;

void setup()
{
  Serial1.begin( 9600 );
  translator.default_setup(orb_buffer.orb_type);

  // set the average gain for all axes to 30
  translator.set_gain( 30 );
  // but set axis 0 to have a gain of 10
  translator.set_axis_gain( 0, 10 );
  // Use sensitivity table 4...
  translator.set_sensitivity_table( sensitivity_4_chart );
  // and enable precision gain for all axes
  translator.set_precision_gain( -20 );
  // ...although oddly make axis 0 MORE sensitive with precision gain!
  translator.set_axis_precision_gain( 0, 30 );
  // and set the precision mask to be hardware button 1
  translator.set_precision_mask( 0x01 );
}

void loop()
{
  orbotron_checkinit( orb_buffer, translator, orb_device );
  orbotron_translate( orb_buffer, translator, orb_device );
}
