#include <SoftwareSerial.h>
#include "serialParser.h"
//#include "timeOut.h"
#include <Adafruit_NeoPixel.h>

#define SET_ADDRESS 3
#define SET_PALETTE 2
//#define SET_NUM_ROWS 4
//#define GET_STATES 1
#define SET_SEQUENCE 1

int ledPin = 0;
int sensorPin = 4;

int prevState = 1;
int setTimer = 0;

SoftwareSerial serial(2, 3); // RX, TX
serialParser parser(serial);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, ledPin, NEO_GRB + NEO_KHZ800);

uint32_t palette[4];
char sequence[16];
int rows = 0;
int current = 0;

unsigned long colorTO = 0;
bool nextColor = false;


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
    strip.setPixelColor(0, strip.Color(0,255,0));
    strip.show();
  });

  parser.on(SET_PALETTE,[](unsigned char * input, int size){
    palette[input[2]] = strip.Color(input[3] * 2, input[4] * 2, input[5] * 2);
    parser.sendPacket(input[0], input[1], input[2],input[3],input[4],input[5]);
    //strip.show();
    //parser.sendPacket(REPORT, input[1], parser.address);
  });

  parser.on(SET_SEQUENCE,[](unsigned char * input, int size){
    rows = input[2];
    for(int i=0; i<rows; i++){
      sequence[i] = (input[3+(i/3)]>>(2*(i%3)))&3;
    }
    parser.sendPacket(REPORT, input[1], sequence[7]);
    current =0;
    strip.setPixelColor(0, palette[sequence[0]]);
    strip.show();
    //parser.sendPacket(REPORT, input[1], parser.address);
  });

  strip.setPixelColor(0, strip.Color(255,0,0));
  strip.show();
}


void loop() // run over and over
{
  parser.idle();
  //idleTimers();

  if(digitalRead(sensorPin) != prevState) {
    prevState = !prevState;
    if(prevState && current < rows){
      strip.setPixelColor(0, 0);
      strip.show();
      if(current<rows-1){
        colorTO = millis() + 100;
        nextColor = true;
        /*setTimeout([](){
          strip.setPixelColor(0, palette[sequence[current]]);
          strip.show();
        }, 100);*/
      }
      //newDetection = true;
    }
  }

  if(colorTO < millis() && nextColor){
    current++;
    strip.setPixelColor(0, palette[sequence[current]]);
    strip.show();
    nextColor = false;
  }
}

