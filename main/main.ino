#include <DHT.h>;
#include "FastLED.h"

// heating
#define HEATER_PIN 3
#define TEMP_SENSOR_PIN 9
#define DHT_TYPE DHT22
#define MAX_TEMP_THRESHOLD 23.89 // 75* fahrenheit
#define MIN_TEMP_THRESHOLD 21.11 // 70* fahrenheit
DHT tempSensor(TEMP_SENSOR_PIN, DHT_TYPE);
float currTemp; // current temperature

// lighting
#define RGB_PIN 6
#define RGB_TYPE WS2812B
#define RGB_ORDER GRB
#define RGB_COUNT 60
#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 50
#define SATURATION 255
CRGB rgbLeds[RGB_COUNT];
int r = MAX_BRIGHTNESS;
int g = 100;
int b = MAX_BRIGHTNESS;

// test
#define TEST_LED 10


void setup() {

  FastLED.addLeds<RGB_TYPE, RGB_PIN, RGB_ORDER>(rgbLeds, RGB_COUNT);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(TEST_LED, OUTPUT);

  cli(); // stop interrupts
 
  // set Timer 1 interrupt at 0.25Hz (4 seconds)
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 0.25Hz increments
  OCR1A = 62499; // = (16*10^6) / (0.25*1024) - 1 (<65536 required for Timer 1 (16-bit))
  // turn on CTC mode (continuous mode?)
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei(); // start interrupts
}

// Timer 1 interrupt 0.25Hz reads enclosure's temp, heats if under 70* celsius
ISR(TIMER1_COMPA_vect){
  
  tempSensor.begin();
  currTemp = tempSensor.readTemperature();

  // if heater: ON {leave on}
  // if heater: OFF {turn on}
  if (currTemp < MIN_TEMP_THRESHOLD) {
    
    // keep heater on
    if (digitalRead(HEATER_PIN)) {
      return;
    }

    // turn heater on
    else {
      digitalWrite(TEST_LED, HIGH);
      digitalWrite(HEATER_PIN, HIGH);
      return;
    }
  }

  // if heater: ON {keep on if under desired MAX_TEMP or turn off if at desired MAX_TEMP}
  // if heater: OFF {keep off}
  else {

    // heater on
    if (digitalRead(HEATER_PIN)) {
      
      // keep heater on
      if (currTemp < MAX_TEMP_THRESHOLD) {
        return;
      }
      
      // turn heater off
      else {
        digitalWrite(TEST_LED, LOW);
        digitalWrite(HEATER_PIN, LOW);
      }
    }

    // keep heater off
    else {
      return;
    }
    
  }
}

void displayWave() {

  if (digitalRead(HEATER_PIN)) {
    return;
  }

  for (int j = 1; j < 65; j++) {
    for (int i = 0; i < RGB_COUNT; i++) {
      rgbLeds[i] = CHSV(i + 115 + (j*1.3), MAX_BRIGHTNESS, SATURATION);
    }
    FastLED.show();
    delay(75);
  }
  
  if (digitalRead(HEATER_PIN)) {
    return;
  }

  for (int j = 65; j >= 1; j--) {
    for (int i = RGB_COUNT; i >= 0; i--) {
      rgbLeds[i] = CHSV(i + 115 + (j*1.3), MAX_BRIGHTNESS, SATURATION);
    }
    FastLED.show();
    delay(75);
  }
}

void setSameColorAndDisplay(int red, int green, int blue) {
  
  for (int i = 0; i < RGB_COUNT; i++) {
    rgbLeds[i] = CRGB(red, green, blue);
    
  }
  FastLED.show();
  
  return;
}

void loop() {

  // heater is on
  while(digitalRead(HEATER_PIN)) {

    for (int red = MIN_BRIGHTNESS; red < MAX_BRIGHTNESS; red++) {
      setSameColorAndDisplay(red, 0, 0);
    }
    for (int red = MAX_BRIGHTNESS; red >= MIN_BRIGHTNESS; red--) {
      setSameColorAndDisplay(red, 0, 0);
    }
  }

  // heater is off
  displayWave();
  
}
