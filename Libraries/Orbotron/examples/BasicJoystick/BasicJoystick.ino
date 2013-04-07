#include "orb_device.h"
#include "orb_translator.h"
#include "hid_keys.h"
#include "chart_4.h"

//change value below to SpaceOrb360, SpaceBall4000, or SpaceBall5000
Logical_orb orb_buffer( SpaceOrb360 );
Orb_translator translator;

void setup()
{
  Serial.begin( 9600 );

  translator.default_setup(orb_buffer.orb_type);
}

void loop()
{
  orbduino_checkinit( orb_buffer, translator, orb_device );
  orbduino_translate( orb_buffer, translator, orb_device );
}

