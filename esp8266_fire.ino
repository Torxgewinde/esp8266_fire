/*
 * Fire Effekt for a strand of eight WS2812b pixels
 * 
 * based on Fire2012 by Mark Kriegsman
 * 
 * uses NeoPixelBus (UART branch) and runs on ESP8266 (nodeMCU 0.9)
 * 
 */

#include <NeoPixelBus.h>

#define NUM_LEDS 8
NeoPixelBus strip = NeoPixelBus(NUM_LEDS, 0);
    
void setup() {
  Serial.begin(115200);
  Serial.println("Compiled at Time: " __TIME__ " Date: " __DATE__);
    
  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();
}


void loop() {
  Fire2015(40, 14, 25); // run simulation frame   
  strip.Show();
  delay(50);
}

// Fire2015 is derived from Fire2012
// it is customized to look like a candle with a strand of eight WS2812b pixels
//
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//
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

void Fire2015(int cooling, int min_heat, int max_heat) {
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i], random(0, ((cooling * 7) / NUM_LEDS) + 1));
  }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 1; k--) {
      heat[k] = (heat[max(0, k - 1)] + heat[max(0, k - 2)]) / 3;
    }
    
    // Step 3.  Heat up at the bottom of the candle
    heat[0] = qadd8( heat[0], random(min_heat,max_heat) );

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      RgbColor color = HeatColor(heat[j]);
      strip.SetPixelColor(j, color);
    }
}



