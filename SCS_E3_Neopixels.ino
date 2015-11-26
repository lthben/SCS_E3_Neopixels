/* Author: Benjamin Low (benjamin.low@digimagic.com.sg)
 *
 * Description: Prototype for SCS E3 RFID station NeoPixels.
 * 
 *   There are three strips to light three different RFID reader stations on the table. 
 *   There are three 7cm cubes with three RFID tags each. There are three different
 *   colours to represent each RFID reader station. For each reader, if there is no 
 *   cube detected, it will pulsate its respective colour. If the wrong cube is placed
 *   on it, the strip will turn red. If the right cube is placed on it, the strip will
 *   turn a solid colour for that station instead of pulsating. The commands for the 
 *   strips are read from the serial port. 
 *
 *   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
 *   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
 *   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
 *   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
 *
 * IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
 * pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
 * and minimize distance between Arduino and first pixel.  Avoid connecting
 * on a live circuit...if you must, connect GND first.
 *
 * Last updated: 25 Nov 2015
 */

#include <Adafruit_NeoPixel.h>

enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, PULSE };
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:

    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
    {
      OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {
      if ((millis() - lastUpdate) > Interval) // time to update
      {
        lastUpdate = millis();
        switch (ActivePattern)
        {
          case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
          case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
          case COLOR_WIPE:
            ColorWipeUpdate();
            break;
          case SCANNER:
            ScannerUpdate();
            break;
          case FADE:
            FadeUpdate();
            break;
          case PULSE:
            PulseUpdate();
            break;
          default:
            break;
        }
      }
    }

    // Increment the Index and reset at the end
    void Increment()
    {
      if (Direction == FORWARD)
      {
        Index++;
        if (Index >= TotalSteps)
        {
          Index = 0;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else // Direction == REVERSE
      {
        --Index;
        if (Index <= 0)
        {
          Index = TotalSteps - 1;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }

    // Reverse pattern direction
    void Reverse()
    {
      if (Direction == FORWARD)
      {
        Direction = REVERSE;
        Index = TotalSteps - 1;
      }
      else
      {
        Direction = FORWARD;
        Index = 0;
      }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_CYCLE;
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
      }
      show();
      Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = THEATER_CHASE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if ((i + Index) % 3 == 0)
        {
          setPixelColor(i, Color1);
        }
        else
        {
          setPixelColor(i, Color2);
        }
      }
      show();
      Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = COLOR_WIPE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color;
      Index = 0;
      Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
      setPixelColor(Index, Color1);
      show();
      Increment();
    }

    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
      ActivePattern = SCANNER;
      Interval = interval;
      TotalSteps = (numPixels() - 1) * 2;
      Color1 = color1;
      Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if (i == Index)  // Scan Pixel to the right
        {
          setPixelColor(i, Color1);
        }
        else if (i == TotalSteps - Index) // Scan Pixel to the left
        {
          setPixelColor(i, Color1);
        }
        else // Fading tail
        {
          setPixelColor(i, DimColor(getPixelColor(i)));
        }
      }
      show();
      Increment();
    }

    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = FADE;
      Interval = interval;
      TotalSteps = steps;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
      // Calculate linear interpolation between Color1 and Color2
      // Optimise order of operations to minimize truncation error
      uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
      uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
      uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

      ColorSet(Color(red, green, blue));
      show();
      Increment();
    }

    //Initialise for a pulse
    void Pulse(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = PULSE;
      Interval = interval;
      TotalSteps = steps;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    //update the pulse pattern
    void PulseUpdate()
    {
      FadeUpdate();
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
      // Shift R, G and B components one bit to the right
      uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, color);
      }
      show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
      return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.

    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }
}; //end of class definition

// ------------------------------------------------------
// Function prototypes for completion callback routines
// ------------------------------------------------------
void Strip1Complete();
void Strip2Complete();
void Strip3Complete();

// -----------------------------
// Object declarations
// -----------------------------
NeoPatterns Strip1(300, 2, NEO_GRB + NEO_KHZ800, &StripComplete);
NeoPatterns Strip2(300, 4, NEO_GRB + NEO_KHZ800, &StripComplete);
NeoPatterns Strip3(300, 6, NEO_GRB + NEO_KHZ800, &StripComplete);

//------------------------------
// setup
//------------------------------
void setup() {
  Serial.begin(9600);

  Strip1.begin();
  Strip2.begin();
  Strip3.begin();
}

//-------------------------
// The main loop
//-------------------------

void loop() {

  Strip1.Update();
  Strip2.Update();
  Strip3.Update();

  read_from_serial();
}

//-----------------------
// supporting functions
//-----------------------

void read_from_serial() {

  unsigned char incomingbyte = 0;

  if (Serial.available() > 0) {

    incomingbyte = Serial.read();

    if (incomingbyte == '0') { //turn all off

      Strip1.ActivePattern = NONE;
      Strip1.ColorSet(Strip1.Color(0, 0, 0));

      Strip2.ActivePattern = NONE;
      Strip2.ColorSet(Strip2.Color(0, 0, 0));

      Strip3.ActivePattern = NONE;
      Strip3.ColorSet(Strip3.Color(0, 0, 0));

    } else if (incomingbyte == '1') { //pulse the first strip yellow

      Strip1.Pulse(Strip1.Color(0, 0, 0), Strip1.Color(127, 127, 0), 50, 10);

    } else if (incomingbyte == '2') { //pulse the second strip green

      Strip2.Pulse(Strip2.Color(0, 0, 0), Strip2.Color(0, 127, 0), 50, 10);

    } else if (incomingbyte == '3') { //pulse the third strip blue

      Strip3.Pulse(Strip3.Color(0, 0, 0), Strip3.Color(0, 0, 127), 50, 10);

    } else if (incomingbyte == '4') { //turn first strip red

      Strip1.ActivePattern = NONE;
      Strip1.ColorSet(Strip1.Color(127, 0, 0));

    } else if (incomingbyte == '5') { //turn second strip red

      Strip2.ActivePattern = NONE;
      Strip2.ColorSet(Strip2.Color(127, 0, 0));

    } else if (incomingbyte == '6') { //turn third strip red

      Strip3.ActivePattern = NONE;
      Strip3.ColorSet(Strip3.Color(127, 0, 0));

    } else if (incomingbyte == '7') { //turn first strip solid yellow

      Strip1.ActivePattern = NONE;
      Strip1.ColorSet(Strip1.Color(127, 127, 0));

    } else if (incomingbyte == '8') { //turn second strip solid green

      Strip2.ActivePattern = NONE;
      Strip2.ColorSet(Strip2.Color(0, 127, 0));

    } else if (incomingbyte == '9') { //turn third strip solid blue

      Strip3.ActivePattern = NONE;
      Strip3.ColorSet(Strip3.Color(0, 0, 127));

    }
  }
}

// -----------------------------
// Completion callback routines
// -----------------------------

void StripComplete() {
  if (Strip1.ActivePattern == PULSE) {
    Strip1.Reverse();
  }
  if (Strip2.ActivePattern == PULSE) {
    Strip2.Reverse();
  }
  if (Strip3.ActivePattern == PULSE) {
    Strip3.Reverse();
  }
}








