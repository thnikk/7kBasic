#include <Arduino.h>
// Universal
#include <Bounce2.h>
#include <HID-Project.h>
#include <FastLED.h>

#define numkeys 8
const byte pins[] = { 2, 3, 7, 9, 10, 11, 12, 4 };
int mapping[] = { 115, 100, 102, 106, 107, 108, 32, 96 };

Bounce * bounce = new Bounce[numkeys+1];

bool pressed[numkeys+1];
bool lastPressed[numkeys+1];

byte b = 127;
byte bMax = 255;

CRGBArray<numkeys> leds;

// Millis timer for idle check
unsigned long pm;

void setup() {
    Serial.begin(9600);

    // Set up LEDs
    FastLED.addLeds<NEOPIXEL, 5>(leds, numkeys);
    FastLED.setBrightness(255);

    // Set pullups and debounce
    for (byte x=0; x<=numkeys; x++) {
        pinMode(pins[x], INPUT_PULLUP);
        bounce[x].attach(pins[x]);
        bounce[x].interval(8);
    }

    // Start keyboard lib
    NKROKeyboard.begin();

}

unsigned long checkMillis;
// All inputs are processed into pressed array
void checkState() {
    // Limiting input polling to polling rate increases speed by 17x!
    // This has no impact on efficacy while making it stupid fast.
    if ((millis() - checkMillis) >= 1) {
        // Write bounce values to main pressed array
        for(byte x=0; x<=numkeys; x++){
        bounce[x].update();
        pressed[x] = bounce[x].read();
        if (pressed[x] == 0) pm = millis();
        }
        // Get current touch value and write to main array
        checkMillis=millis();
    }
}

// Compares inputs to last inputs and presses/releases key based on state change
void keyboard() {
    for (byte x=0; x<=numkeys; x++){
        // If the button state changes, press/release a key.
        if ( pressed[x] != lastPressed[x] ){
            if (!pressed[x]) NKROKeyboard.press(mapping[x]);
            if (pressed[x]) NKROKeyboard.release(mapping[x]);
            lastPressed[x] = pressed[x];
        }
    }
}

byte plusVal=255/numkeys;
void wheel(){
    static uint8_t hue;
    bool facePress = 0;
    for(int i = 0; i < numkeys; i++) {
        if (pressed[i]) leds[i] = CHSV(hue+(plusVal*i),255,255);
        else {
            leds[i] = 0xFFFFFF;
            facePress = 1;
        }
    }
    hue++;
    FastLED.show();
}

unsigned long effectMillis;
byte effectSpeed = 2;
void effects(byte speed) {
    // All LED modes should go here for universal speed control
    if ((millis() - effectMillis) > speed){
        wheel();

        if (b < bMax) b++;
        if (b > bMax) b--;
        FastLED.setBrightness(b);
        effectMillis = millis();
    }
}

unsigned long speedCheckMillis;
int count;
void speedCheck() {
    count++;
    if ((millis() - speedCheckMillis) > 1000){
        Serial.println(count);
        count = 0;
        speedCheckMillis = millis();
    }
}


void idle(){
    if ((millis() - pm) > 60000) bMax = 0;
    else bMax = 255;
}

void loop() {
    // Update key state to check for button presses
    checkState();
    // Convert key presses to actual keyboard keys
    keyboard();
    // Make lights happen
    effects(10);
    idle();
    // Debug check for loops per second
    speedCheck();

    //serialCheck();
}

