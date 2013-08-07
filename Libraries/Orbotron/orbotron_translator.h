#ifndef ORBOTRON_TRANSLATOR_H
#define ORBOTRON_TRANSLATOR_H

#define NUM_LOGICAL_AXES 6
#define NUM_LOGICAL_BUTTONS 16

#include <arduino.h>
#define NUM_POSSIBLE_AXIS_KEYBINDINGS 6
#include <string.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "hid_keys.h"
#define INRANGE( angle, min, max ) ( ( angle >= min ) && (angle <= max ) )

struct Axis_key_binding
{
  unsigned short axis;
  unsigned short min_value;
  unsigned short max_value;
  unsigned char modifiers;
  unsigned char key;
};

struct Button_key_binding
{
  unsigned short button_mask;
  unsigned char modifier;
  unsigned char key;
};

#define MOUSE_AXIS_X 1
#define MOUSE_AXIS_Y 2
#define MOUSE_AXIS_WHEEL 3

struct Axis_mouse_binding
{
  unsigned short axis;
  unsigned short mouse_axis;
  short scale;
  short absolute_motion_radius;
  short absolute_motion_scale;
};

struct Button_mouse_binding
{
  unsigned short button_mask;
  unsigned char mouse_button;
};

struct Fourway_keyboard_binding
{
  unsigned short axis_x;
  unsigned short axis_y;
  
  char n;
  char s;
  char e;
  char w;

  long deadzone_radius_squared;

};

const PROGMEM Fourway_keyboard_binding WASD_fourway_binding = { 0, 1, KEY_S, KEY_W, KEY_D, KEY_A, 75*75 };
const PROGMEM Fourway_keyboard_binding ESDF_dvorak_fourway_binding = { 0, 1, KEY_E, KEY_PERIOD, KEY_O, KEY_U, 75*75 };

const PROGMEM unsigned char Axis_map_spaceorb_face[]= {0,1,2,3,4,5};
const PROGMEM short Polarity_spaceorb_face[] = {1,1,1,1,1,1};
const PROGMEM unsigned char Axis_map_spaceball_4k[] = {0,2,1,3,5,4};
const PROGMEM short Polarity_spaceball_4k[] = {1,-1,-1,1,-1,-1};
const PROGMEM short Polarity_spaceball_5k[] = {1,1,-1,1,-1,-1};
unsigned short sensitivity_chart( short value, const unsigned short * ptable )
{
  if ( value < 0 )
    {
      return 0;
    }
  else if ( ( value >= 0 ) && ( value <= 1023 ) )
  {
    return pgm_read_word(ptable+value);
  }
  else
  {
    return 1023;
  }
}

short bounded_number( short x, 
		      short min_val,
		      short max_val )
{
  if ( x <= min_val ) 
    {
      return min_val;
    }
  else if ( x >= max_val )
    {
      return max_val;
    }
  else 
    {
      return x;
    }
}

unsigned short 
axis_with_gain( short base, int polarity, short gain, const unsigned short * ptable )
{
  short result;
  // use polarity
  if ( polarity < 0 )
    {
      base = -base;
    }

/*   // use gain */
/*   if ( gain < 0 ) */
/*     { */
/*       base = (base*10)/(-gain); */
/*     } */
/*   else if ( gain > 0 ) */
/*     { */
/*       base = base * gain / 10; */
/*     } */

  // okay, we've scaled base by gain.  Now cap
  base = bounded_number(base + 512,0,1023);

  if ( ptable != NULL )
    {
      result = sensitivity_chart( base, ptable ) - 512;
    }
  else 
    {
      result = base - 512;
    }
  // now use gain
  if ( gain < 0 )
    {
      result = (result*10)/(-gain);
    }
  else if ( gain > 0 )
    {
      result = result * gain / 10;
    }
  result = bounded_number( result+512,0,1023);
  return result;
}

//mouse interval in milliseconds
#define DEFAULT_MOUSE_INTERVAL 10

class Orbotron_translator// : public Device_translator
{
public:

  bool chording;

  Button_key_binding* button_key_bindings;
  unsigned short num_button_key_bindings;
  Axis_key_binding* axis_key_bindings;
  unsigned short num_axis_key_bindings;
  Axis_mouse_binding* axis_mouse_bindings;
  unsigned short num_axis_mouse_bindings;
  Button_mouse_binding* button_mouse_bindings;
  unsigned short num_button_mouse_bindings;
  const Fourway_keyboard_binding* fourway_bindings;
  unsigned short num_fourway_bindings;
  const unsigned short* p_sensitivity_table;
  short gain[NUM_LOGICAL_AXES];
  short precision_gain[NUM_LOGICAL_AXES];
  unsigned short precision_mask;
  unsigned long last_mouse_update_time;
  unsigned short last_mouse_orb_values[2];
  unsigned short translated_axes[ NUM_LOGICAL_AXES ];
  bool send_joystick_reports;
  const unsigned char *axis_map;
  // polarity is by LOGICAL axis
  const short *polarity;
  short mouse_interval;

  Orbotron_translator( void ) :
  chording( false ),
    button_key_bindings( NULL ),
    num_button_key_bindings( 0 ),
    axis_key_bindings( NULL ),
    num_axis_key_bindings( 0 ),
    axis_mouse_bindings( NULL ),
    num_axis_mouse_bindings( 0 ),
    button_mouse_bindings( NULL ),
    num_button_mouse_bindings( 0 ),
    fourway_bindings( NULL ),
    num_fourway_bindings( 0 ),
    p_sensitivity_table( NULL  ),
    precision_mask(0),
    last_mouse_update_time( 0),
    send_joystick_reports(true),
    axis_map( Axis_map_spaceorb_face ),
    polarity( Polarity_spaceorb_face ),
    mouse_interval( DEFAULT_MOUSE_INTERVAL )
  {
    axis_map = Axis_map_spaceorb_face;
    polarity = Polarity_spaceorb_face;
    last_mouse_orb_values[0] = 0;
    last_mouse_orb_values[1] = 0;

    set_gain(0);
    set_precision_gain(0);
    for ( int i = 0; i < NUM_LOGICAL_AXES; ++i )
      {
	translated_axes[i]=512;
      }
    precision_mask = 0;
  }

  void set_sensitivity_table( const unsigned short* p_new_sensitivity_table )
    {
      p_sensitivity_table = p_new_sensitivity_table;
    }

  void set_gain( short new_gain )
    {
      for ( int i = 0; i < NUM_LOGICAL_AXES; ++i )
	{
	  gain[i] = new_gain;
	}
    }

  void set_axis_gain( int axis, short new_gain)
    {
      gain[axis] = new_gain;
    }

  void set_precision_gain( short new_precision_gain )
    {
      for ( int i = 0; i < NUM_LOGICAL_AXES; ++i )
	{
	  precision_gain[i] = new_precision_gain;
	}
    }

  void set_axis_precision_gain( int axis, short new_precision_gain )
    {
      precision_gain[axis] = new_precision_gain;
    }

  void set_precision_mask( unsigned short new_precision_mask )
    {
      precision_mask = new_precision_mask;
    }

  void set_axis_key_bindings( Axis_key_binding* p_new_binding, int num_bindings )
    {
      axis_key_bindings = p_new_binding;
      num_axis_key_bindings = num_bindings;
    }

  void set_axis_mouse_bindings( Axis_mouse_binding* p_new_binding, int num_bindings )
    {
      //presently you can only have 2 bindings; technically more is possible but
      //would mildly complicate the code for last_mouse_orb_values
      if (num_bindings > 2) 
	{
	  num_bindings = 2;
	}
      axis_mouse_bindings = p_new_binding;
      num_axis_mouse_bindings = num_bindings;
    }

  void set_button_key_bindings( Button_key_binding* p_new_bindings, int num_bindings )
    {
      button_key_bindings = p_new_bindings;
      num_button_key_bindings = num_bindings;
    }

  void set_button_mouse_bindings( Button_mouse_binding* p_new_bindings, int num_bindings )
    {
      button_mouse_bindings = p_new_bindings;
      num_button_mouse_bindings = num_bindings;
    }

  void set_fourway_bindings( const Fourway_keyboard_binding* p_new_bindings, int num_bindings )
    {
      fourway_bindings  = p_new_bindings;
      num_fourway_bindings = num_bindings;
    }

  void set_chording( bool new_chording )
  {
    chording = new_chording;
  }

  void set_polarity( const short* p_new_polarity )
  {
    polarity = p_new_polarity;
  }

  void set_axis_map( const unsigned char* p_new_map )
  {
    axis_map = p_new_map;
  }

  void default_setup( OrbType orb_type )
    {
      switch( orb_type )
	{
	case SpaceOrb360 :
	  set_chording( true );
	  set_axis_map( Axis_map_spaceorb_face );
	  set_polarity( Polarity_spaceorb_face );
	  send_joystick_reports = true;
	  break;
	case SpaceBall4000 :
	  set_chording( false );
	  set_axis_map( Axis_map_spaceball_4k );
	  set_polarity( Polarity_spaceball_4k );
	  send_joystick_reports = true;
	  break;
	case SpaceBall5000 :
	  set_chording( false );
	  set_axis_map( Axis_map_spaceball_4k );
	  set_polarity( Polarity_spaceball_5k );
	  set_axis_gain( 0, 20 );
	  set_axis_gain( 1, 20 );
	  set_axis_gain( 2, 20 );

	  set_axis_gain( 3, 20 );
	  set_axis_gain( 4, 20 );
	  set_axis_gain( 5, 20 );
	  send_joystick_reports = true;
	  break;
	}
    }
  
  unsigned short scaled_axis_value( short physical_val, int polarity )
  {
    // takes a value from 0 to 1023 and returns the sensitivity chart version
    switch( polarity )
    {
    case 1 : 
      return physical_val; //sensitivity_chart( physical_val );
      break;
    case 0 :
      return 0;
      break;
    case -1 :
      return 1023-physical_val; //sensitivity_chart( 1023 - physical_val );
      break;
    }

  }

  unsigned short mapped_buttons( Logical_orbotron& orb ) 
  {
    unsigned short result;
    // work chording here
    if ( chording )
    {
      int chord_page = orb.physical_buttons & 3;
      result = ( orb.physical_buttons >> 2 ) << ( 4*chord_page);
    }
    else
    {
      result = orb.physical_buttons;// & 63;
    }
    return result;
  }

  unsigned short mapped_axis( Logical_orbotron& orb, int index )
  {
    short this_gain = ( orb.physical_buttons & precision_mask ) ? precision_gain[index] : gain[index];
    if ( orb.orb_type == SpaceOrb360 )
      {
	return axis_with_gain( orb.axis( pgm_read_byte(&axis_map[index]) ), (int)(pgm_read_word(&polarity[index])), this_gain, p_sensitivity_table );
      }
    else
      {
        // for the spaceball, we have a problem-- axes can be very large and not particularly bounded (confusing).
        // this means we have to artificially bound and scale for safety reasons
        short ax = orb.axis(pgm_read_byte(&axis_map[index])); // >> ( index < 3 ? 2 : 0 );
	ax = bounded_number( ax, -512, 511 );
        return axis_with_gain( ax, (int)(pgm_read_word(&polarity[index])), this_gain, p_sensitivity_table );
      }
  }

  void translate_keyboard_bindings( unsigned short *buffer, Logical_orbotron& from, Orbotron_device& to )
    {
#define KEYPRESS_BUFFER_LENGTH 7
      byte keypresses[KEYPRESS_BUFFER_LENGTH];
      //byte 0 will be modifiers; all other bytes are simultaneous keys
      memset( keypresses, 0, KEYPRESS_BUFFER_LENGTH );
      int key_cursor = 1;
      unsigned short buttons = mapped_buttons(from);
      for ( unsigned int i = 0; i < num_button_key_bindings; ++i )
	{
	  unsigned short mask =pgm_read_word( &button_key_bindings[i].button_mask );
	  if ( (buttons & mask) == mask )
	    {
	      unsigned char b = pgm_read_byte( &button_key_bindings[i].modifier );
	      if ( b != 0 )
		{
		  keypresses[0] |= b;
		}
	      b = pgm_read_byte( &button_key_bindings[i].key );
	      if ( (b != 0 ) && (key_cursor < KEYPRESS_BUFFER_LENGTH) )
		{
		  keypresses[key_cursor] = b;
		  key_cursor++;
		}
	    }
	}

      // now add axis key mappings
       for ( unsigned short j = 0; j < num_axis_key_bindings; ++j ) 
 	{ 
	  unsigned short ax = buffer[ pgm_read_word(&axis_key_bindings[j].axis) ];
	  if ( ( key_cursor < 6 ) 
	       && ( ax >= pgm_read_word(&axis_key_bindings[j].min_value ))
	       && ( ax <= pgm_read_word(&axis_key_bindings[j].max_value )) )
	    {
	      if ( pgm_read_byte(&axis_key_bindings[j].modifiers) )
		{
		  keypresses[0] |= pgm_read_byte(&axis_key_bindings[j].modifiers);
		}
	      else if ( key_cursor < KEYPRESS_BUFFER_LENGTH )
		{
		  keypresses[key_cursor] = pgm_read_byte(&axis_key_bindings[j].key);
		  key_cursor++;
		}
	    }
	}
       // now add fourway-bindings
       for ( unsigned short k = 0; k < num_fourway_bindings; ++k )
	 {
	   long x = (int)(buffer[pgm_read_word(&(fourway_bindings[k].axis_x))]) - 512;
	   long y = (int)(buffer[pgm_read_word(&(fourway_bindings[k].axis_y))]) - 512;
  
	   float angle = atan2( y,x );
#define EMAX_ANGLE 1.178
#define EMIN_ANGLE -1.178
#define NMIN_ANGLE 0.3927
#define NMAX_ANGLE 2.7489
#define WMIN_ANGLE 1.9635
#define WMAX_ANGLE -1.9635
#define SMIN_ANGLE -0.3927
#define SMAX_ANGLE -2.7489
	   // now create the keyboard report depending on the angle.  Each direction covers a
	   // 135 degree swath which overlaps, so we basically divide into 8 octants where
	   // the overlaps have both pressed.
	   if ( (x*x + y*y) > (long)(pgm_read_dword(&(fourway_bindings[k].deadzone_radius_squared ))) )
	     {
	       if ( INRANGE( angle, NMIN_ANGLE, NMAX_ANGLE ) && (key_cursor < KEYPRESS_BUFFER_LENGTH) )
		 {
		   keypresses[key_cursor] = pgm_read_byte(&(fourway_bindings[k].n));
		   ++key_cursor;
		 }
	       if ( INRANGE( angle, SMAX_ANGLE, SMIN_ANGLE ) && (key_cursor < KEYPRESS_BUFFER_LENGTH) )
		 {
		   keypresses[key_cursor] = pgm_read_byte(&(fourway_bindings[k].s));
		   ++key_cursor;
		 }
	       if (( INRANGE( angle, 0, EMAX_ANGLE ) || INRANGE( angle, EMIN_ANGLE, 0 ) ) && (key_cursor < KEYPRESS_BUFFER_LENGTH))
		 {
		   keypresses[key_cursor] = pgm_read_byte(&(fourway_bindings[k].e));
		   ++key_cursor;
		 }
	       if (( INRANGE( angle, WMIN_ANGLE, 3.15 ) || INRANGE( angle, -3.15, WMAX_ANGLE ) )  && (key_cursor < KEYPRESS_BUFFER_LENGTH) )
		 {
		   keypresses[key_cursor] = pgm_read_byte(&(fourway_bindings[k].w));
		   ++key_cursor;
		 }
	     }
	 }
       to.send_keyboard_report( keypresses[0], 5, keypresses+1 );
    }

  void translate_joystick( unsigned short *buffer, 
			   Logical_orbotron& from, 
			   Orbotron_device& to )
  {
    for ( int i = 0; i < NUM_LOGICAL_AXES; ++i )
      {
	buffer[i] = mapped_axis( from, i );
      }
  }

  char scaled_mouse_relative_move( short orb_axis,
				   short scale,
				   short absolute_motion_radius )
    {
      short ax = 0;
      short low = 512-absolute_motion_radius;
      short high = 512+absolute_motion_radius;
      // if the orb reading is in the absolute motion radius, return 0
      if ( ( low < orb_axis ) 
	   && ( orb_axis < high ) )
	{
	  return 0;
	}
      // clip axes to the motion outside the absolute radius
      else if ( orb_axis <= low )
	{
	  orb_axis = orb_axis - low;
	}
      else if ( orb_axis >= high ) 
	{
	  orb_axis = orb_axis-high;
	}
      if ( scale < 0 ) 
	{
	  ax = bounded_number(-orb_axis >> (-scale), -127, 127);
	}
      else
	{
	  ax = bounded_number(orb_axis >> (scale), -127, 127);
	}
      return ax;
    }

  char scaled_mouse_absolute_move( short orb_axis,
				   short scale,
				   short absolute_motion_radius,
				   short last_orb_value )
  {
    // this isn't trivial--if we go suddenly into or out of the absolute motion area
    // we don't want to have the part outside the area count
    short ax = bounded_number(orb_axis, 
			      512-absolute_motion_radius,
			      512+absolute_motion_radius) 
      - bounded_number( last_orb_value, 
			512-absolute_motion_radius,
			512+absolute_motion_radius );
    // if we're outside the absolute motion area, don't have any absolute move
    if ( (ax < -absolute_motion_radius) && (absolute_motion_radius < ax) )
      {
	ax = 0;
      }
    if ( scale < 0 )
      {
    	ax = bounded_number( -ax >> (-scale), -127, 127);
      }
    else
      {
    	ax = bounded_number( ax >> (scale), -127, 127);
      }
    return ax;
  }

  void translate_mouse_bindings( unsigned short* buffer,
				 Logical_orbotron& from,
				 Orbotron_device& to )
    {
      unsigned char mouse_buttons = 0;
      char x = 0;
      char y = 0;
      char wheel=0;
      for ( unsigned int i = 0; i < num_axis_mouse_bindings; ++i )
	{
	  unsigned short ax = buffer[ pgm_read_word(&axis_mouse_bindings[i].axis) ];
	  char mouse_ax =  
	    scaled_mouse_relative_move( ax, 
					pgm_read_word( &axis_mouse_bindings[i].scale ),
					pgm_read_word( &axis_mouse_bindings[i].absolute_motion_radius )) 
	    + scaled_mouse_absolute_move( ax, 
					  pgm_read_word( &axis_mouse_bindings[i].absolute_motion_scale ),
					  pgm_read_word( &axis_mouse_bindings[i].absolute_motion_radius ),
					  last_mouse_orb_values[i] );
	  last_mouse_orb_values[i] = ax;
	  switch (pgm_read_byte( &axis_mouse_bindings[i].mouse_axis ))
	    {
	    case MOUSE_AXIS_X :
	      x = mouse_ax;
	      break;
	    case MOUSE_AXIS_Y:
	      y = mouse_ax;
	      break;
	    case MOUSE_AXIS_WHEEL :
	      wheel = mouse_ax;
	      break;
	    }
	}

      // now we have a trick with mouse buttons, because if we send
      // too many button reports, it thinks we're aggressively double-clicking
      unsigned short buttons = mapped_buttons(from);
      for ( unsigned int i = 0; i < num_button_mouse_bindings; ++i )
	{
	  unsigned short mask =pgm_read_word( &button_mouse_bindings[i].button_mask );
	  if ( (buttons & mask) == mask )
	    {
	      mouse_buttons |= pgm_read_byte( &button_mouse_bindings[i].mouse_button );
	    }
	}

      // do button bindings
      to.send_mouse_report( mouse_buttons, x, y, wheel );
    }

  void update( Logical_orbotron& from, Orbotron_device& to )
  {
    translate_joystick( translated_axes, from, to );
/*     if ( (num_axis_mouse_bindings > 0) || (num_button_mouse_bindings > 0) ) */
/*       { */
/* 	translate_mouse_bindings( translated_axes, from, to ); */
/*       } */

    from.log_change();
  }
};

extern "C" long unsigned int millis( void );

inline void
orbotron_checkinit( Logical_orbotron& from, Orbotron_translator& translator, Orbotron_device& to )
{
  if ( (!(to.init_string_sent)) && (millis() - to.millis_at_init > 2000) ) //from.physical_buttons & 1) )
    {
      if (from.orb_type == SpaceBall4000)
	{
	  //to.safe_send_serial_P(spaceball_setup_string);
	  Serial1.print("\rCB\rNT\rFT?\rFR?\rP@r@r\rMSSV\rZ\rBcCcCcC\r");
	}
      else if ( from.orb_type == SpaceBall5000 )
	{
	  Serial1.print("\rm3\rpBB\rz\r");
	  //not sure why this isn't working any more, incidentally
	  //to.safe_send_serial_P(magellan_setup_string);
	}
      to.init_string_sent = true;
    }
  
}

inline void
orbotron_translate( Logical_orbotron& from, Orbotron_translator& translator, Orbotron_device& to )
{
  int n=Serial1.available();
  while ( n > 0 )
    {
      byte b = Serial1.read();
      from.add_byte(b);
      --n;
    }
  if ( from.has_changed() )
    {
      translator.update( from, to );
      if ( translator.send_joystick_reports )
	{
	      //translator.translate_joystick( buffer, orb_buffer, Orbotron_device );
/* 	      to.send_joystick_report( 7, 15, 31, 63, 127, 255, 7 ); */
	  to.send_joystick_report( translator.translated_axes[0],
				   translator.translated_axes[1],
				   translator.translated_axes[2],
				   translator.translated_axes[3],
				   translator.translated_axes[4],
				   translator.translated_axes[5],
				   translator.mapped_buttons(from) );
	}
    }
  if ( (translator.num_button_key_bindings > 0)
       || (translator.num_axis_key_bindings > 0)
       || (translator.num_fourway_bindings > 0))
    {
      translator.translate_keyboard_bindings( translator.translated_axes, from, to );
    }
  unsigned long current_time = millis();
  if ( (translator.num_axis_mouse_bindings > 0 )
       || (translator.num_button_mouse_bindings > 0 ) )
    {
      if ( current_time > translator.last_mouse_update_time + translator.mouse_interval )
	{
	  translator.translate_mouse_bindings( translator.translated_axes, from, to );
	  translator.last_mouse_update_time = current_time;
	}
    }
  
}

void
orbotron_loopfunc( Logical_orbotron& from, Orbotron_translator& translator, Orbotron_device& to )
{
  orbotron_checkinit( from, translator, to );
  orbotron_translate( from, translator, to );
}

#endif
