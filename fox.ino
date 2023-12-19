#include <HardwareSerial.h>
#include <ArduinoJson.h>
#define _DICT_USE_PSRAM
#define _DICT_COMPRESS_SHOCO
#include <Dictionary.h>

#define SOUND_PIN 3 //D1 (GPIO3)
#define PTT_PIN 5 //D3 (GPIO5)
#define PD_PIN 6 //D4 (GPIO6)
#define HL_PIN 7 //D5 (GPIO7)
#define INITIAL_DELAY 1000
#define MAX_TX_VOLUME 8
#define MAX_MELODY_SIZE 128

struct note {
  String note;
  int duration;
};

Dictionary &noteDictionary = *(new Dictionary(90));
HardwareSerial transceiver(0); //TX: D6 (GPIO27), RX: D7 (GPIO26)
note melody[MAX_MELODY_SIZE];

float txFrequency = 146.565;

//Frequency range
//UHF 400-480 MHz
//VHF 134-174 MHz
//multiple of 25kHz

int txVolume = 5; // Volume 1-8
int secondsBetweenTransmissions = 10;
int tempo = 140;
int melodySize = 0;

void setup() {  
  transceiver.begin(9600, SERIAL_8N1, -1, -1);
  Serial.begin(115200);

  //TODO
  //morse = createMorse(callmessage);
  pinMode(PTT_PIN, OUTPUT);
  pinMode(PD_PIN, OUTPUT);
  pinMode(HL_PIN, OUTPUT);

  loadMelody();
  loadNoteDictionary();
  setTransceiverFrequency();
  setTransceiverVolume();
  setPowerLevelLow();
  enableTxMode();
}

void loop() {
    enableTransceiver();
    
    transmitMelody();
    //delay(750);
    //playMorse();
    
    disableTransceiver();
    delay(secondsBetweenTransmissions * 1000);
}

void loadNoteDictionary() {
  //TODO load from file
  String noteFrequencyMapping = "{\"REST\":0,\"B0\":31,\"C1\":33,\"CS1\":35,\"D1\":37,\"DS1\":39,\"E1\":41,\"F1\":44,\"FS1\":46,\"G1\":49,\"GS1\":52,\"A1\":55,\"AS1\":58,\"B1\":62,\"C2\":65,\"CS2\":69,\"D2\":73,\"DS2\":78,\"E2\":82,\"F2\":87,\"FS2\":93,\"G2\":98,\"GS2\":104,\"A2\":110,\"AS2\":117,\"B2\":123,\"C3\":131,\"CS3\":139,\"D3\":147,\"DS3\":156,\"E3\":165,\"F3\":175,\"FS3\":185,\"G3\":196,\"GS3\":208,\"A3\":220,\"AS3\":233,\"B3\":247,\"C4\":262,\"CS4\":277,\"D4\":294,\"DS4\":311,\"E4\":330,\"F4\":349,\"FS4\":370,\"G4\":392,\"GS4\":415,\"A4\":440,\"AS4\":466,\"B4\":494,\"C5\":523,\"CS5\":554,\"D5\":587,\"DS5\":622,\"E5\":659,\"F5\":698,\"FS5\":740,\"G5\":784,\"GS5\":831,\"A5\":880,\"AS5\":932,\"B5\":988,\"C6\":1047,\"CS6\":1109,\"D6\":1175,\"DS6\":1245,\"E6\":1319,\"F6\":1397,\"FS6\":1480,\"G6\":1568,\"GS6\":1661,\"A6\":1760,\"AS6\":1865,\"B6\":1976,\"C7\":2093,\"CS7\":2217,\"D7\":2349,\"DS7\":2489,\"E7\":2637,\"F7\":2794,\"FS7\":2960,\"G7\":3136,\"GS7\":3322,\"A7\":3520,\"AS7\":3729,\"B7\":3951,\"C8\":4186,\"CS8\":4435,\"D8\":4699,\"DS8\":4978}";
  noteDictionary.jload(noteFrequencyMapping);
}

void loadMelody() {
  StaticJsonDocument<4096> doc;
  //TODO load from file
  //todo change keys to 'n' and 'd'
  char json[] = "{\"melody\":[{\"note\":\"A4\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"D5\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"FS5\",\"duration\":-8},{\"note\":\"FS5\",\"duration\":-8},{\"note\":\"E5\",\"duration\":-4},{\"note\":\"A4\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},\r\n{\"note\":\"D5\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"E5\",\"duration\":-8},{\"note\":\"E5\",\"duration\":-8},{\"note\":\"D5\",\"duration\":-8},{\"note\":\"CS5\",\"duration\":16},{\"note\":\"B4\",\"duration\":-8},{\"note\":\"A4\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},\r\n{\"note\":\"D5\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"D5\",\"duration\":4},{\"note\":\"E5\",\"duration\":8},{\"note\":\"CS5\",\"duration\":-8},{\"note\":\"B4\",\"duration\":16},{\"note\":\"A4\",\"duration\":8},{\"note\":\"A4\",\"duration\":8},{\"note\":\"A4\",\"duration\":8},\r\n{\"note\":\"E5\",\"duration\":4},{\"note\":\"D5\",\"duration\":2},{\"note\":\"A4\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"D5\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"FS5\",\"duration\":-8},{\"note\":\"FS5\",\"duration\":-8},{\"note\":\"E5\",\"duration\":-4},\r\n{\"note\":\"A4\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"D5\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"A5\",\"duration\":4},{\"note\":\"CS5\",\"duration\":8},{\"note\":\"D5\",\"duration\":-8},{\"note\":\"CS5\",\"duration\":16},{\"note\":\"B4\",\"duration\":8},\r\n{\"note\":\"A4\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"D5\",\"duration\":16},{\"note\":\"B4\",\"duration\":16},{\"note\":\"D5\",\"duration\":4},{\"note\":\"E5\",\"duration\":8},{\"note\":\"CS5\",\"duration\":-8},{\"note\":\"B4\",\"duration\":16},{\"note\":\"A4\",\"duration\":4},\r\n{\"note\":\"A4\",\"duration\":8},{\"note\":\"E5\",\"duration\":4},{\"note\":\"D5\",\"duration\":2},{\"note\":\"REST\",\"duration\":4}]}";
  
  deserializeJson(doc, json);
  
  for (JsonObject melody_item : doc["melody"].as<JsonArray>()) {
    if (melodySize > MAX_MELODY_SIZE) {
      break;
    }
    
    melody[melodySize].note = melody_item["note"].as<String>();
    melody[melodySize].duration = melody_item["duration"].as<signed int>();
    melodySize++;
  }
}

void enableTransceiver() {
  digitalWrite(PD_PIN, HIGH);
  delay(1000);
}

void disableTransceiver() {
  digitalWrite(PD_PIN, LOW);
}

void enableTxMode() {
  digitalWrite(PTT_PIN, LOW);
}

void setPowerLevelLow() {
  digitalWrite(HL_PIN, LOW);
}

void setTransceiverFrequency() {
  transceiver.printf("AT+DMOSETGROUP=1,%.4f,%.4f,0000,3,0000\r\n", txFrequency, txFrequency);
  delay(100);
}

void setTransceiverVolume() {
  transceiver.printf("AT+DMOSETVOLUME=%d\r\n", min(txVolume, MAX_TX_VOLUME));
  delay(100);
}

void transmitMelody() {
  int wholenote = (60000 * 4) / tempo;
  int divider = 0;
  int noteDuration = 0;

  for (int thisNote = 0; thisNote < melodySize; thisNote++) {
    // calculates the duration of each note
    divider = melody[thisNote].duration;
    if (divider > 0) {
      // Regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // Dotted notes are represented with negative durations
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(SOUND_PIN, noteDictionary[melody[thisNote].note].toInt(), noteDuration * 0.9);

    // Wait for the specific duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(SOUND_PIN);
  }
}
