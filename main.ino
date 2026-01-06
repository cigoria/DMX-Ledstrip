#include <FastLED.h>
#include <Conceptinetics.h>

// --------- KONFIG ---------
#define LED_PIN 6
#define NUM_LEDS 120      // 2m WS2813 @ 60LED/m
#define DMX_START 1       // DMX kezdő cím

//#define MODE_B        // csak az egyiket hagyd!
#define MODE_A

CRGB leds[NUM_LEDS];

// DMX beállítás
#ifdef MODE_A
#define DMX_CHANNELS 6
#else
#define DMX_CHANNELS 5
#endif

DMX_Slave dmx_slave(DMX_CHANNELS);

// --------- EFFEKT SEGÉDVÁLTOZÓK ---------
uint8_t rainbowHue = 0;
uint8_t fadeValue = 0;
bool fadeDir = true;
unsigned long lastStep = 0;

// --------- SEGÉDFÜGGVÉNYEK ---------
void showColor(uint8_t r, uint8_t g, uint8_t b, uint8_t master)
{
  for(int i=0;i<NUM_LEDS;i++)
  {
    leds[i] = CRGB(
      (r * master) / 255,
      (g * master) / 255,
      (b * master) / 255
    );
  }
  FastLED.show();
}

void rainbowEffect()
{
  for(int i=0;i<NUM_LEDS;i++)
  {
    leds[i] = CHSV((rainbowHue + (i * 5)) % 255, 255, 255);
  }
  rainbowHue++;
  FastLED.show();
}

void instantChange()
{
  static uint8_t colorIndex = 0;
  uint8_t colors[7][3] = {
    {255,0,0},
    {0,255,0},
    {0,0,255},
    {255,255,0},
    {0,255,255},
    {255,0,255},
    {255,255,255}
  };

  showColor(colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2], 255);
  colorIndex = (colorIndex + 1) % 7;
}

void fadeChange()
{
  if (fadeDir) fadeValue++;
  else fadeValue--;

  if (fadeValue == 255) fadeDir = false;
  if (fadeValue == 0) fadeDir = true;

  CHSV color(rainbowHue, 255, fadeValue);
  for(int i=0;i<NUM_LEDS;i++) leds[i] = color;
  rainbowHue++;
  FastLED.show();
}

// ----------------- SETUP -----------------
void setup()
{
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  dmx_slave.enable();
  dmx_slave.setStartAddress(DMX_START);
}

// ----------------- LOOP ------------------
void loop()
{
#ifdef MODE_A
  uint8_t R = dmx_slave.getChannelValue(1);
  uint8_t G = dmx_slave.getChannelValue(2);
  uint8_t B = dmx_slave.getChannelValue(3);
  uint8_t Master = dmx_slave.getChannelValue(4);
  uint8_t Strobe = dmx_slave.getChannelValue(5);
  uint8_t Mode = dmx_slave.getChannelValue(6);

  // Strobe
  static unsigned long strobeTimer = 0;
  if(Strobe > 0)
  {
    unsigned long interval = map(Strobe, 1, 255, 200, 20);
    if(millis() - strobeTimer > interval)
    {
      static bool on = false;
      on = !on;
      if(on) showColor(R,G,B,Master);
      else FastLED.clear(), FastLED.show();
      strobeTimer = millis();
    }
    return;
  }

  // Mode Effektek
  if(Mode > 0 && millis() - lastStep > 30)
  {
    if(Mode < 85) rainbowEffect();
    else if(Mode < 170) instantChange();
    else fadeChange();

    lastStep = millis();
    return;
  }

  // Normál RGB + Master
  showColor(R,G,B,Master);

#endif


#ifdef MODE_B
  uint8_t segment = dmx_slave.getChannelValue(1);
  uint8_t R = dmx_slave.getChannelValue(2);
  uint8_t G = dmx_slave.getChannelValue(3);
  uint8_t B = dmx_slave.getChannelValue(4);
  uint8_t Master = dmx_slave.getChannelValue(5);

  int ledIndex = map(segment, 0, 255, 0, NUM_LEDS-1);
  for(int i=0;i<NUM_LEDS;i++)
  {
    if(i <= ledIndex)
      leds[i] = CRGB((R*Master)/255,(G*Master)/255,(B*Master)/255);
    else
      leds[i] = CRGB::Black;
  }
  FastLED.show();
#endif
}
