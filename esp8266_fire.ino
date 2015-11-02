/*
 * Fire Effekt for a strand of eight WS2812b pixels
 * 
 * based on Fire2012
 * 
 * uses NeoPixelBus (UART branch) and runs on ESP8266 (nodeMCU 0.9)
 * 
 */

#include <NeoPixelBus.h>

#define NUM_LEDS 8
NeoPixelBus strip = NeoPixelBus(NUM_LEDS, 0);
    
void setup()
{
    Serial.begin(115200);
    Serial.println("ESP starting up... Time: " __TIME__ " Date: " __DATE__);
    
    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();
}


void loop()
{
    Fire2012(0); // run simulation frame   
    strip.Show();
    delay(50);
}

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  40

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 200

#define max(a,b) ((a)>(b)?(a):(b))

uint8_t qsub8( uint8_t i, uint8_t j) {
    int t = i - j;
    if( t < 0) t = 0;
    return t;
}

uint8_t qadd8( uint8_t i, uint8_t j) {
    unsigned int t = i + j;
    if( t > 255) t = 255;
    return t;
}

uint8_t scale8_video( uint8_t i, uint8_t scale)
{
    uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
}

RgbColor LinearBlend(RgbColor left, RgbColor right, uint8_t progress)
{
  return RgbColor( left.R + ((right.R - left.R) * progress / 255),
    left.G + ((right.G - left.G) * progress / 255),
    left.B + ((right.B - left.B) * progress / 255));
}

typedef struct {RgbColor color; uint8_t fulcrum;} COLOR_AND_FULCRUM;

RgbColor HeatColor(uint8_t temperature)
{
    RgbColor heatcolor;

    // at least two colors, sorted by fulcrum, min max fulcrums at 0 an 255
    COLOR_AND_FULCRUM fireColors[] = {
      {
        RgbColor(0, 0, 0),
        0
      },
      {
        RgbColor(25, 10, 0),
        40
      },
      {
        RgbColor(255, 110, 0),
        160
      },
      {
        RgbColor(255, 120, 0),
        220
      },
      {
        RgbColor(255, 70, 0),
        255
      }
    };
    
    int fireColorsLength = sizeof(fireColors) / sizeof(COLOR_AND_FULCRUM);

    // find adjacent colors for current value
    int i;
    for (i=0; i<fireColorsLength; i++) {
      if( fireColors[i].fulcrum > temperature ) break;
    }

    // determine how far to blend
    /*
    Serial.print("temp: " + String(temperature) + \
                 ", Left fulcrum: " + String(fireColors[i-1].fulcrum) +\
                 ", Right fulcrum: " + String(fireColors[i].fulcrum) +\
                 ", blend: " + String((temperature - fireColors[i-1].fulcrum) * 255/(fireColors[i].fulcrum - fireColors[i-1].fulcrum)) + "\n");
    */
    return LinearBlend(fireColors[i-1].color, fireColors[i].color, (temperature - fireColors[i-1].fulcrum) * 255/(fireColors[i].fulcrum - fireColors[i-1].fulcrum));
}

void Fire2012(uint8_t dim)
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i], random(0, ((COOLING * 7) / NUM_LEDS) + 1));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 1; k--) {
      heat[k] = (heat[max(0, k - 1)] + heat[max(0, k - 2)]) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    /*if( random(255) < SPARKING ) {
      int y = random(1);
      heat[y] = qadd8( heat[y], random(0,150) );
    }*/
    heat[0] = qadd8( heat[0], random(14,25) );

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      RgbColor color = HeatColor(heat[j]);
      color.Darken(dim);
      strip.SetPixelColor(j, color);
    }
}



