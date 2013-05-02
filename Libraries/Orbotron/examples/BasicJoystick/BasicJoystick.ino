#include <orbotron_device.h>
#include <orbotron_buffer.h>
#include <orbotron_translator.h>
#include <chart_3.h>
// change "SpaceOrb360" to 
// SpaceBall4000 for SpaceBall 4000FLX devices
// SpaceBall5000 for SpaceBall 5000 or Magellan SpaceMouse devices

Logical_orbotron orb_buffer( SpaceOrb360 );
Orbotron_translator translator;
Orbotron_device orb_device;

void setup()
{
  Serial1.begin(9600);
  translator.default_setup(orb_buffer.orb_type);
  translator.set_sensitivity_table( sensitivity_3_chart );
}


void loop()
{
  orbotron_checkinit( orb_buffer, translator, orb_device );
  orbotron_translate( orb_buffer, translator, orb_device );
}
