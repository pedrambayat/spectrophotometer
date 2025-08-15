#include <Adafruit_LiquidCrystal.h>
Adafruit_LiquidCrystal lcd(0);

// constants
const int v_out = A0; // photoresistor pin
#define ON_OFF_BUTTON 9
#define MEASURE_BUTTON 5
#define LED_PIN 12
#define BUZZER 11
const int r_1 = 977; // ~1 kOhm resistor
const float v_in = 5.0; // Vin

// calibration values
double b_1 = -12.759523; // slope
double b_0 = -0.0122514; // y-intercept

// measurement values
double r2_blank = 4241; // to be hardcoded
double conc, s_ca, interval;

// regression metrics
double s_r = 0.01909888; // standard error
const int m = 100; // number of measurements to take
double measurements[m];
int n = 4; // number of values in regression
double s_std = -0.2036443; // average of regression absorbance data
double s_cstd = 0.01290994; // stdev of calibration concentrations

// states
bool isOn = false; // LCD starts off
bool lastSwitchState = HIGH;
// bool isFirstSample = false;

// measurement button states
int ss; // turns on once the button is pressed
int pss = 0; // previous switch state

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

int onOffread = 0;
int measureRead = 0;


// tunes

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

int onMelody[] = {
  NOTE_C6, 16, NOTE_G5, 16, NOTE_D6, 16, NOTE_B5, 16

};

int notesOn = sizeof(onMelody) / sizeof(onMelody[0]) / 2;
int tempoOn = 144;

int offMelody[] = {
  NOTE_A4,4

};

int notesOff = sizeof(offMelody) / sizeof(offMelody[0]) / 2;
int tempoOff = 120;

int healthyMelody[] = {
  // Max Verstappen theme song
  NOTE_A4,-8, NOTE_A4,-8, NOTE_A4,8, NOTE_E5,2,
  REST,8, NOTE_E5,4, NOTE_E5,8, NOTE_C4,4, NOTE_B4,4,
  NOTE_A4,-8, NOTE_A4,-8, NOTE_A4,8, NOTE_E5,2,
  REST,8, NOTE_E5,4, NOTE_E5,8, NOTE_C4,4, NOTE_B4,4

};

int notesHealthy = sizeof(healthyMelody) / sizeof(healthyMelody[0]) / 2;
int tempoHealthy = 128;

int diseaseMelody[] = {
  NOTE_A5,4, NOTE_C4,4, NOTE_E4,4
};

int notesDisease = sizeof(diseaseMelody) / sizeof(diseaseMelody[0]) / 2;
int tempoDisease = 120;

void setup() {
  Serial.begin(9600);

  // pin modes
  pinMode(ON_OFF_BUTTON, INPUT_PULLUP);
  pinMode(MEASURE_BUTTON, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // LCD
  lcd.begin(16,2);
  lcd.setBacklight(HIGH);
}

void loop() {
  ss = digitalRead(MEASURE_BUTTON);
  int reading = digitalRead(ON_OFF_BUTTON);

  if (reading != lastSwitchState) {
    lastDebounceTime = millis(); // reset the debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Only toggle if state actually changed
    if (reading != isOn) {
      isOn = reading;

      if (isOn == HIGH) {
        // Switch turned ON
        lcd.setBacklight(HIGH);
        lcd.display();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("insert sample");
//        isFirstSample = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Switch turned ON");
        playMelody(onMelody, notesOn, tempoOn);
      } else {
        // Switch turned OFF
        lcd.setBacklight(LOW);
        lcd.noDisplay();
        digitalWrite(LED_PIN, LOW);
        Serial.println("Switch turned OFF");
        playMelody(offMelody, notesOff, tempoOff);
      }
    }
  }
  lastSwitchState = reading;

  // measure a sample
  if (ss == 1 && pss == 0) { // button pressed and was prev not pressed
    // populate measurement array with 3 photoresistor output

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("reading...");

    for (int i = 0; i <= m; i++) { 
      measurements[i] = analogRead(v_out);
    }

    // calculate the average of the photoresistor readings as output
    double sum = 0.0;
    for (int i = 0; i <= m; i++) { // calculate the sum
      sum += measurements[i];
    }
    double output = (sum / m) * 0.0049; // calculate v_out
    
    // calculations
//    double output = analogRead(v_out) * 0.0049;
    double absorbance = log10(r2_blank) - log10((r_1) * (output / (v_in - output)));
    conc = (absorbance - b_0) / b_1;
    s_ca = (s_r / b_1) * pow(((1/m) + (1/n) + (pow((absorbance-s_std),2) / (b_1*b_1*s_cstd*s_cstd*(n-1)))), 0.5);
    interval = abs(2.920 * s_ca);
    double lower_bound = conc - interval;
    double upper_bound = conc + interval;
    
    // display result
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(String(conc * 100) + " +/- " + String(interval * 100));
    lcd.setCursor(0,1);

    if (conc >= 0.01 && lower_bound >= 0.01) {
      lcd.print("DISEASE!");
      lcd.setCursor(9,1);
      lcd.print(absorbance);
      lcd.setCursor(0,1);
      playMelody(diseaseMelody, notesDisease, tempoDisease);
    } else if ((conc >= 0.01 && lower_bound <= 0.01) || (conc <= 0.01 && upper_bound >= 0.01)) {
      lcd.print("UNCERTAIN!");
    } else if (conc <= 0.01 && upper_bound <= 0.01) {
      lcd.print("healthy :)");
      playMelody(healthyMelody, notesHealthy, tempoHealthy);
    }
    Serial.println("sample");
    
  Serial.println("v_out: " + String(output));
  Serial.println("absorbance: " + String(absorbance));
  Serial.println("conc: " + String(conc));
  delay(5000); // show the result for 5 seconds 
  }
  pss = ss;

}

void playMelody(const int* melody, int length, int tempo) {
  // iterates over the notes of the melody
  // array length is 2 * notes since has durations as well
  int divider = 0, noteDuration = 0;
  int wholenote = (60000 * 4) / tempo;

  for (int i = 0; i < length * 2; i += 2) {

    // duration of each note
    divider = melody[i + 1];

    if (divider > 0) {
      // regular note, just proceed
      noteDuration = wholenote / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations
      noteDuration = (wholenote / abs(divider)) * 1.5;
    }
    
    // play the note for 90% of its length, 10% is a pause
    int note = melody[i];
    if (note != REST) {
      tone(BUZZER, note, noteDuration * 0.9);
    }
    delay(noteDuration);
    noTone(BUZZER);
  }
}
