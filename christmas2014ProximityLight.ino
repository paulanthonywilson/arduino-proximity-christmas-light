#include <NewPing.h>
#include <NewTone.h>

#define TONE_PIN 12

const int melody[] = { 262, 196, 196, 220, 196, 0, 247, 262 };
const int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };


const int triggerPin = 8;
const int echoPin = 7;

NewPing sonar(triggerPin, echoPin, 300);

const int amberDistance = 120;
const int alarmDistance = 50;


#define RED 9
#define GREEN 10
#define BLUE 11


void setup() {
  Serial.begin(9600);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(TONE_PIN, INPUT);
  randomSeed(analogRead(0));
  randomColour();

}

void writeColour(char colour_pin, int rgb_code) {
  analogWrite(colour_pin, 255 - (rgb_code % 256));
}


void setColour(int r, int g, int b) {
  writeColour(RED, r);
  writeColour(GREEN, g);
  writeColour(BLUE, b);
}

void green() {
  setColour(0, 255, 0);
}

void red() {
  setColour(255, 0, 0);
}

void blue() {
  setColour(0, 0, 255);
}

void amber() {
  setColour(255, 255, 0);
}

void randomColour() {
  long colour = random(0, 16777216);
  int r = colour / 65536;
  int g = ((colour - r) / 256) % 256;
  int b = colour % 256;
  setColour(r, g, b);
}

/*
Applies a low pass filter to prevent anomalous readings triggering the alarm,
particularly when voltage is low
*/
const double filteringFactor = 0.1;
double filteredProximity = 500.0;
long getProximity() {
  long echoTime = sonar.ping_median();
  double proximity = (double) sonar.convert_cm(echoTime);
  filteredProximity = (proximity * filteringFactor) +
                      (filteredProximity * (1 - filteringFactor));
  return (long) filteredProximity;
}

boolean alarmSounded = false;
void alarm() {
  if (alarmSounded) return;
  red();
  Serial.println("alarm");
  for (unsigned long freq = 125; freq <= 15000; freq += 10) {
    NewTone(TONE_PIN, freq); // Play the frequency (125 Hz to 15 kHz sweep in 10 Hz steps).
    delay(1); // Wait 1 ms so you can hear it.
  }
  noNewTone(TONE_PIN); // Turn off the tone.
  for (int thisNote = 0; thisNote < 8; thisNote++) { // Loop through the notes in the array.
    int noteDuration = 1000 / noteDurations[thisNote];
    NewTone(TONE_PIN, melody[thisNote], noteDuration); // Play thisNote for noteDuration.
    delay(noteDuration * 4 / 3); // Wait while the tone plays in the background, plus another 33% delay between notes.
  }
  delay(500);
  pinMode(TONE_PIN, INPUT); // Stop tone playing when colour changes
  alarmSounded = true;
}

void amberAlert() {
  alarmSounded = false;
  amber();
}

long lastColourChange = 0;
long colourChangeDuration = 1000;
void allClear() {
  alarmSounded = false;
  if (millis() - lastColourChange > colourChangeDuration) {
    lastColourChange = millis();
    randomColour();
    colourChangeDuration = random(0, 3000);
  }
}

void loop() {
  long proximity = getProximity();
  Serial.println(proximity);
  if (proximity < alarmDistance && proximity > 0) {
    alarm();
  } else if (proximity < amberDistance) {
    amberAlert();
  } else {
    allClear();
  }
}
