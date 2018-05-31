#include <SoftwareSerial.h>
#include "serialParser.h"
#include "timeOut.h"
#include <Adafruit_NeoPixel.h>

#define SET_ADDRESS 3
#define SET_COLOR 2
#define GET_STATES 1

int ledPin = 0;
int sensorPin = 4;

int newDetection = 0;
int prevState = 0;
int setTimer = 0;
int clearTimer = -1;

SoftwareSerial serial(2, 3); // RX, TX
serialParser parser(serial);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, ledPin, NEO_GRB + NEO_KHZ800);


void setup()
{
  // set the data rate for the SoftwareSerial port
  serial.begin(19200);

  pinMode(sensorPin,INPUT);
  strip.begin();
  strip.show();

  parser.setWrongAddrCB([](unsigned char * input, int size){
    int tot = 0;
    for(int i=0; i<size; i++){
      serial.write(input[i]);
      tot+=input[i];
    }
    serial.write(tot & 0b01111111);
    serial.write(START_FLAG + STOP_FLAG);
  });

  parser.on(SET_ADDRESS,[](unsigned char * input, int size){
    parser.address = input[2] + 1;
    parser.sendPacket(input[0], input[1], parser.address);
  });

  parser.on(SET_COLOR,[](unsigned char * input, int size){
    strip.setPixelColor(0, input[2] * 2, input[3] * 2, input[4] * 2);
    strip.show();
    parser.sendPacket(REPORT, input[1], parser.address);
  });

  parser.on(GET_STATES,[](unsigned char * input, int size){
    int numBoards = input[2];
    int tot = (START_FLAG | BROADCAST) + GET_STATES + numBoards;
    serial.write(START_FLAG | BROADCAST);
    serial.write(GET_STATES);
    serial.write(numBoards);

    for(int j = 0; j  < (numBoards / 7) + 1; j++){
      char state = input[3 + j];
      if(j == (parser.address - 1) / 7){
        bitWrite(state, (parser.address - 1) % 7, newDetection);
      }
  
      tot+= state;
      serial.write(state);
    }
  
    serial.write(tot & 0b01111111);
    serial.write(START_FLAG + STOP_FLAG);

    newDetection = 0;
  });

  strip.setPixelColor(0, strip.Color(255,0,0));
  strip.show();
}


void loop() // run over and over
{
  parser.idle();
  idleTimers();

  if(digitalRead(sensorPin) != prevState) {
    prevState = !prevState;
    if(prevState){
      newDetection = true;
    }
  }
}

