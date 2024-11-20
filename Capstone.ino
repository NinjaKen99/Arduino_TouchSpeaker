// Library Imports
#include <SD.h>                 // Library for SD card reading
#include <TMRpcm.h>             // Library for asynchronous playback of PCM/WAV files
#include <SPI.h>                // Need to include the SPI library
#include <CapacitiveSensor.h>   // Library for pressure sensor
#include <light_CD74HC4067.h>   // Library for specific multiplexer

////// Define fixed variables //////
//// Pin Numbers
constexpr int SD_ChipSelectPin  = 10;   // connect pin 10 of arduino to cs pin of sd card
constexpr int SpeakerPin        = 9;    // 5,6,11 or 46 on Mega, 9 on Uno, Nano, etc
constexpr int Sensor_Input      = 4;    // pin 4 sends electrical energy to sensor
constexpr int SelectPin0        = 3;    // Multiplexer s0 select pin
constexpr int SelectPin1        = 5;    // Multiplexer s1 select pin
constexpr int SelectPin2        = 6;    // Multiplexer s2 select pin
constexpr int SelectPin3        = 7;    // Multiplexer s3 select pin
// List of analog pins for multiplexer
constexpr int myPins[] = {A0};

//// Number Values
constexpr int VolumeLevel       = 6;    //Volume level for speaker
constexpr int PressureThreshold = 350;  // Pressure from sensor for positive result 500-700 and above, no more than 1000
constexpr int SensePeriod       = 10;  // How long to sense each sensor for (milliseconds)
//// Test audio name (to delete at end)
const char* testfilename        = "testing.wav"; // audio to test speaker

////// Declare global variables ////// (Will be used repeatedly)
CD74HC4067 mux(SelectPin0, SelectPin1, SelectPin2, SelectPin3); // Multiplexer object with select pins
TMRpcm tmrpcm;            // Create object for speaker
char filename[8];           // Variable for audio file name

//// Struct
struct sensor_pin {
  int mult;       // analog pin to multiplexer?
  byte mult_pin;  // Select pin byte for multiplexer
  bool signal;    // Bool if signal found
};


void setup() {
  Serial.begin(9600);
  //// Set up SD card reading and audio setup
  Serial.println("Beginning Initialisation...");

  // Initialise SD card
  Serial.print("Initialising SD card...   ");
  if (!SD.begin(SD_ChipSelectPin)) // returns 1 if the card is present
  {
    Serial.println("Error: SD card not present or CS/SS pin is wrong.");
    return;
  } else {
    Serial.println("SD card initialised.");
  }

  // Initialise Speaker
  Serial.print("Initialising Speaker...   ");
  tmrpcm.speakerPin = SpeakerPin;
  tmrpcm.setVolume(VolumeLevel); // Initialise audio volume
  Serial.println("Speaker initialised.");

  Serial.print("Testing Speaker...   ");
  tmrpcm.play(testfilename); // the sound file "song" will play each time the arduino powers up, or is reset
                            // try to provide the file name with extension

  if (!tmrpcm.isPlaying()) { // Checks if audio in progress
    Serial.println("Speaker did not play setup audio.");
  } else {
    Serial.println("Speaker played setup audio.");
  }
  while (tmrpcm.isPlaying()); //Holds code until audio done
  Serial.println("Setup Complete.");
}


void loop() {
  // Create and reset sensor instance
  sensor_pin sensor = {-1, 0, false};
  // Check for sensor signal
  sensor = check_signal(sensor);
  // If sensor found with signal
  if (sensor.signal == true) {
    Serial.println("Signal detected.");
    // Play audio file
    play_audio(sensor.mult, sensor.mult_pin);
  }
}

// Function to check for sensor signal
sensor_pin check_signal(sensor_pin input) {
  Serial.println("Checking for signal.");
  // loop through analog pins connected to multiplexers
  for (int pin: myPins) {
    // Read from selected multiplexer
    CapacitiveSensor capSensor = CapacitiveSensor(Sensor_Input, pin);
    // Loop through multiplexer pins connected to sensors
    for (byte i = 14; i < 16; i++) {
      // Read from selected pins
      mux.channel(i);
      // Check sensor for period of time
      for (int interval = 0; interval < SensePeriod; interval++) {
        // Checks value from sensor
        // long sensorValue = capSensor.capacitiveSensor(30);
        long sensorValue = analogRead(pin);
        Serial.println(sensorValue);
        // If signal found (Compare value against threshold)
        if (sensorValue >= PressureThreshold) {
          input.mult = pin - 14; // Formating for Analog pins
          input.mult_pin = i + 1; // Format to range 1-16
          input.signal = true;
          return input;
        }
        delay(1);
      }
    }
    // // Check sensor for period of time
    // for (int interval = 0; interval < SensePeriod; interval++) {
    //   // Checks value from sensor
    //   long sensorValue = capSensor.capacitiveSensor(30);
    //   Serial.println(sensorValue);
    //   // If signal found (Compare value against threshold)
    //   if (sensorValue >= PressureThreshold) {
    //     // Serial.print(sensorValue);
    //     // Serial.println("   Signal Found");
    //     // Store values
    //     input.mult = pin;
    //     input.mult_pin = 0b0000; // change later
    //     input.signal = true;
    //     return input;
    //   }
    //   // Pause for millisecond
    //   delay(1);
    // }
  }
}

// Function to play audio file
void play_audio (int number, byte id) {
  //// byte doesn't matter for now
  // Safeguard for potential errors
  if (number >= 0) {
    // Format filename string with associated number, eg. 001.wav
    Serial.print("Getting audio file: ");
    Serial.print(number);
    Serial.println(id);
    sprintf(filename, "%03d.wav", (number)*16 + (id-12));
    // Serial.println(filename);
    // Start playing associated audio file from sd card
    Serial.println("Playing audio file.");
    tmrpcm.play(filename);
    // Pause loop until audio done
    while (tmrpcm.isPlaying());
    Serial.println("Audio finished.");
  }
}

