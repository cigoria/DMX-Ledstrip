#include <FastLED.h>
#include <DMXSerial.h>

// --------- KONFIG ---------
#define LED_PIN 6
#define NUM_LEDS 60         // 1m * 60 LED/m

#define PIN_ADDR_UP 7
#define PIN_ADDR_DOWN 8

#define PIN_SWITCH_A 9
#define PIN_SWITCH_B 10

CRGB leds[NUM_LEDS];

int dmxStartAddress = 1;
bool lastUpState = HIGH;
bool lastDownState = HIGH;

// --------- Effekt változók ---------
uint8_t rainbowHue = 0;
uint8_t fadeValue = 0;
bool fadeDir = true;
unsigned long lastStep = 0;

// --------- Segédfüggvények ---------
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
  uint8_t colors[7][3] =
  {
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
  for(int i=0;i<NUM_LEDS;i++)
    leds[i] = color;

  rainbowHue++;
  FastLED.show();
}

// --------- SETUP ---------
void setup()
{
  pinMode(PIN_SWITCH_A, INPUT);
  pinMode(PIN_SWITCH_B, INPUT);
  pinMode(PIN_ADDR_UP, INPUT_PULLUP);
  pinMode(PIN_ADDR_DOWN, INPUT_PULLUP);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // DMX fogadás
  DMXSerial.init(DMXReceiver);
}

// --------- LOOP ---------
void loop()
{
  // --- DMX Cím Állítás ---
  bool currentUp = digitalRead(PIN_ADDR_UP);
  bool currentDown = digitalRead(PIN_ADDR_DOWN);

  if (currentUp == LOW && lastUpState == HIGH) {
    if (dmxStartAddress < 506) dmxStartAddress++; // Max cím védelem
    delay(50); // Pergésmentesítés
  }
  lastUpState = currentUp;

  if (currentDown == LOW && lastDownState == HIGH) {
    if (dmxStartAddress > 1) dmxStartAddress--;
    delay(50); // Pergésmentesítés
  }
  lastDownState = currentDown;

  if (digitalRead(PIN_SWITCH_A) == HIGH) {
  uint8_t R = DMXSerial.read(dmxStartAddress + 0);
  uint8_t G = DMXSerial.read(dmxStartAddress + 1);
  uint8_t B = DMXSerial.read(dmxStartAddress + 2);
  uint8_t Master = DMXSerial.read(dmxStartAddress + 3);
  uint8_t Strobe = DMXSerial.read(dmxStartAddress + 4);
  uint8_t Mode = DMXSerial.read(dmxStartAddress + 5);

  // ----- Strobe -----
  static unsigned long strobeTimer = 0;
  if(Strobe > 0)
  {
    unsigned long interval = map(Strobe, 1, 255, 200, 20);
    if(millis() - strobeTimer > interval)
    {
      static bool on = false;
      on = !on;
      if(on) showColor(R,G,B,Master);
      else
      {
        FastLED.clear();
        FastLED.show();
      }
      strobeTimer = millis();
    }
    return;
  }

  // ----- Mode Effektek -----
  if(Mode > 0 && millis() - lastStep > 30)
  {
    if(Mode < 85) rainbowEffect();
    else if(Mode < 170) instantChange();
    else fadeChange();

    lastStep = millis();
    return;
  }

  // ---- Normál RGB + Master ----
  showColor(R,G,B,Master);

  } else if (digitalRead(PIN_SWITCH_B) == HIGH) {

  uint8_t segment = DMXSerial.read(dmxStartAddress + 0);
  uint8_t R = DMXSerial.read(dmxStartAddress + 1);
  uint8_t G = DMXSerial.read(dmxStartAddress + 2);
  uint8_t B = DMXSerial.read(dmxStartAddress + 3);
  uint8_t Master = DMXSerial.read(dmxStartAddress + 4);

  int ledIndex = map(segment, 0, 255, 0, NUM_LEDS-1);

  for(int i=0;i<NUM_LEDS;i++)
  {
    if(i <= ledIndex)
      leds[i] = CRGB((R*Master)/255,(G*Master)/255,(B*Master)/255);
    else
      leds[i] = CRGB::Black;
  }

  FastLED.show();
  }
}
