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
constexpr int Sensor_Output     = 2;    // pin 2 senses senses a change in sensor
// List of pins in use attached to sensors TODO: switch to analog pin list for multiplexer
constexpr int myPins[] = {2};
//// Number Values
constexpr int VolumeLevel       = 6;    //Volume level for speaker
constexpr int PressureThreshold = 700; // Pressure from sensor for positive result 190 lowest
constexpr int SensePeriod       = 100;  // How long to sense each sensor for
//// Test audio name (to delete at end)
const char* testfilename        = "testing.wav"; // audio to test speaker

////// Declare global variables ////// (Will be used repeatedly)
TMRpcm tmrpcm;            // Create object for speaker
// CD74HC4067 mux(8, 9, 10, 11); // Multiplexer object with select pins
char cstr[3];             // For formatting file name
char* filename;           // Variable for audio file name

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
  Serial.println(sensor.signal);
  if (sensor.signal == true) {
    Serial.println("Signal detected.");
    // Play audio file
    play_audio(sensor.mult, sensor.mult_pin);
  }
}

// Function to check for sensor signal
sensor_pin check_signal(sensor_pin input) {
  Serial.println("Checking for signal.");
  // Loop through pins connected to sensors
  for (int pin: myPins) {
    // for (byte i = 0; i < 16; i++) {}
    // Read sensor for <time> milliseconds
    CapacitiveSensor capSensor = CapacitiveSensor(Sensor_Input, pin);
    // Check sensor for period of time
    for (int interval = 0; interval < SensePeriod; interval++) {
      // Checks value from sensor
      long sensorValue = capSensor.capacitiveSensor(30);
      Serial.println(sensorValue);
      // If signal found
      if (sensorValue >= PressureThreshold) {
        // Serial.print(sensorValue);
        Serial.println("   Signal Found");
        // Store values
        input.mult = pin;
        input.mult_pin = 0b0000; // change later
        input.signal = true;
        return input;
      }
      // Pause for millisecond
      delay(1);
    }
  }
}

// Function to play audio file
void play_audio (int number, byte id) {
  //// byte doesn't matter for now
  // Safeguard for potential errors
  if (number >= 0) {
    // Format filename string with associated number, eg. 001.wav
    Serial.print("Getting audio file: ");
    Serial.println(number);
    // filename = sprintf(cstr, "%03d", number) + ".wav";
    filename = "002.wav";
    // Start playing associated audio file from sd card
    Serial.println("Playing audio file.");
    tmrpcm.play(filename);
    // Pause loop until audio done
    while (tmrpcm.isPlaying());
    Serial.println("Audio finished.");
    // Clear pointer in case
    filename = nullptr;
  }
}

