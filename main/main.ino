#include <FastLED.h>
#include <DMXSerial.h>
#include <EEPROM.h>


// --------- KONFIG ---------
#define LED_PIN 6
#define NUM_LEDS 60         // 1m * 60 LED/m

#define PIN_ADDR_UP 7
#define PIN_ADDR_DOWN 8

#define PIN_SWITCH_A 9
#define PIN_SWITCH_B 10

#define BINARY_DISPLAY_TIME 3000  // Bináris megjelenítés ideje ms-ban (3 másodperc)

CRGB leds[NUM_LEDS];

uint16_t dmxStartAddress = 1;
bool lastUpState = HIGH;
bool lastDownState = HIGH;
unsigned long lastButtonPress = 0;  // Utolsó gombnyomás ideje

// Gomb nyomva tartás kezeléséhez
unsigned long upPressTime = 0;
unsigned long downPressTime = 0;
unsigned long lastRepeatUp = 0;
unsigned long lastRepeatDown = 0;

// --------- Effekt változók ---------
uint8_t rainbowHue = 0;
uint8_t fadeValue = 0;
bool fadeDir = true;
unsigned long lastStep = 0;

// --------- Segédfüggvények ---------
void showBinaryChannel(int channel)
{
  // Töröljük az első 10 LED-et
  for(int i = 0; i < 10; i++) {
    leds[i] = CRGB::Black;
  }
  
  // Bináris megjelenítés az első 10 LED-en (LSB-től MSB-ig)
  for(int i = 0; i < 10; i++) {
    if(channel & (1 << i)) {
      if (digitalRead(PIN_SWITCH_A) == LOW) {
       leds[i] = CRGB::Red;  // 1 bit = Piros, ha A mód
      }
      else {
       leds[i] = CRGB::Blue;  // 1 bit = Kék, ha B mód
      }
    }
  }
  
  FastLED.show();
}

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
  pinMode(PIN_SWITCH_A, INPUT_PULLUP);
  pinMode(PIN_SWITCH_B, INPUT_PULLUP);
  pinMode(PIN_ADDR_UP, INPUT_PULLUP);
  pinMode(PIN_ADDR_DOWN, INPUT_PULLUP);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  EEPROM.get(0,dmxStartAddress);

  // DMX fogadás
  DMXSerial.init(DMXReceiver);

  // Indításkor 5 mp "Mode 1" jellegű szivárvány
  rainbowHue = 0;
  unsigned long startupStart = millis();
  while (millis() - startupStart < 5000) {
    rainbowEffect();
    delay(30);
  }
  FastLED.clear();
  FastLED.show();
}

// --------- LOOP ---------
void loop()
{
  // --- DMX Cím Állítás ---
  bool currentUp = digitalRead(PIN_ADDR_UP);
  bool currentDown = digitalRead(PIN_ADDR_DOWN);

  unsigned long now = millis();

  if (currentUp == LOW && lastUpState == HIGH) {
    if (dmxStartAddress < 506) dmxStartAddress++; // Max cím védelem
    lastButtonPress = now;  // Gombnyomás időpontja
    upPressTime = now;      // Nyomva tartás kezdete
    lastRepeatUp = now;     // Ismétlés időzítő indul
  }
  // Hosszú nyomás – gyors ismétlés
  if (currentUp == LOW && upPressTime > 0) {
    unsigned long held = now - upPressTime;

    // Első 500 ms: csak 1 lépés (single click érzet)
    if (held > 500) {
      // Minél tovább tartod, annál gyorsabb:
      // 0.5–1 s: lassabb, 1–2 s: közepes, 2 s felett: nagyon gyors
      unsigned long interval;
      if (held > 2000) {
        interval = 50;   // nagyon gyors
      } else if (held > 1000) {
        interval = 100;  // közepes
      } else {
        interval = 200;  // lassabb
      }

      if (now - lastRepeatUp >= interval) {
        if (dmxStartAddress < 506) dmxStartAddress++;
        lastRepeatUp = now;
        lastButtonPress = now;
      }
    }
  }
  // Ha elengedjük a gombot, nullázzuk az állapotot
  if (currentUp == HIGH && lastUpState == LOW) {
    upPressTime = 0;
  }
  lastUpState = currentUp;

  if (currentDown == LOW && lastDownState == HIGH) {
    if (dmxStartAddress > 1) dmxStartAddress--;
    lastButtonPress = now;  // Gombnyomás időpontja
    downPressTime = now;    // Nyomva tartás kezdete
    lastRepeatDown = now;   // Ismétlés időzítő indul
  }
  // Hosszú nyomás – gyors ismétlés
  if (currentDown == LOW && downPressTime > 0) {
    unsigned long held = now - downPressTime;

    if (held > 500) {
      unsigned long interval;
      if (held > 2000) {
        interval = 50;   // nagyon gyors
      } else if (held > 1000) {
        interval = 100;  // közepes
      } else {
        interval = 200;  // lassabb
      }

      if (now - lastRepeatDown >= interval) {
        if (dmxStartAddress > 1) dmxStartAddress--;
        lastRepeatDown = now;
        lastButtonPress = now;
      }
    }
  }
  // Ha elengedjük a gombot, nullázzuk az állapotot
  if (currentDown == HIGH && lastDownState == LOW) {
    downPressTime = 0;
  }
  lastDownState = currentDown;

  // --- Bináris megjelenítés gombnyomás után ---
  if (millis() - lastButtonPress < BINARY_DISPLAY_TIME) {
    showBinaryChannel(dmxStartAddress);
    EEPROM.put(0,dmxStartAddress);
    return;  // Ne futtassuk a normál DMX kódot
  }


  if (digitalRead(PIN_SWITCH_A) == LOW) {
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

  } else if (digitalRead(PIN_SWITCH_B) == LOW) {

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
