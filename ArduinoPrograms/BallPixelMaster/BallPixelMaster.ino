#include <AltSoftSerial.h>
#include "serialParser.h"
#include "timeOut.h"

#define COLUMN_COLOR 64
#define READY 127
#define BALL_DETECT 32
#define NUM_COLUMNS 16

////////// Light strip defines:
#define SET_ADDRESS 3
#define SET_COLOR 2
#define GET_STATES 1


AltSoftSerial serial;

serialParser parser(Serial);
serialParser cells(serial);

bool scheduleRead = 0, scheduleColor = 0, scheduleCount = 1; 
bool writing = false;

char colAddr = 1;
char r = 0;
char g = 127;
char b = 0;

int numColumns = 0;

unsigned long writeTimer = 0;
unsigned long countTimer = 0;

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(115200);
  serial.begin(19200);
  
  parser.address = 1;

  cells.address = REPORT;

  parser.on(READY, [](unsigned char * input, int size){
    parser.sendPacket(REPORT,READY);
  });

  parser.on(COLUMN_COLOR, [](unsigned char * input, int size){
    colAddr = input[2];
    r = input[3];
    g = input[4];
    b = input[5];
    scheduleColor = true;
    //cells.sendPacket(input[2],SET_COLOR, input[3], input[4], input[5]);
  });

  cells.on(SET_COLOR, [](unsigned char * input, int size){
    writing = false;
  });

  cells.on(SET_ADDRESS, [](unsigned char * input, int size){
    numColumns = input[2];
    parser.sendPacket(REPORT, NUM_COLUMNS, numColumns);
    writing = false;
  });

  cells.on(GET_STATES, [](unsigned char * input, int size){
    //numColumns = input[2];
    int curBoard = 0;
    bool changed = false;
    for( int j = 0; j < (numColumns / 7) + 1; j++){
      int nex = input[3 + j];
      
      do {
        int curVal = bitRead(nex,curBoard % 7);
        if(curVal){
          parser.sendPacket(REPORT, BALL_DETECT, curBoard);
        }
                      
        curBoard++;
      } while(curBoard % 7 && curBoard <= numColumns);
    }
    writing = false;
  });

  parser.sendPacket(REPORT,READY);
}

void requestCount(){
  cells.sendPacket(BROADCAST, SET_ADDRESS,0);
}

void setColor(){
  cells.sendPacket(colAddr,SET_COLOR, r,g,b);
}

void getSensorStates(){
  if(numColumns>0){
    int tot = (START_FLAG | BROADCAST) + GET_STATES + numColumns;
    serial.write(START_FLAG | BROADCAST);
    serial.write(GET_STATES);
    serial.write(numColumns);
    
    for(int i=0; i < (numColumns / 7) + 1; i++){
      serial.write((byte)0x00);
    }
  
    serial.write(tot & 0b01111111);
    serial.write(START_FLAG + STOP_FLAG);
  }
}

bool* schedules[3] = {&scheduleCount, &scheduleColor, &scheduleRead};
void (* callbacks [3])() = {requestCount, setColor, getSensorStates};

void loop() {
  parser.idle();
  cells.idle();

  if(countTimer < millis()){
    countTimer = millis() + 200;
    scheduleRead = true;
  }

  if(writeTimer < millis() && writing){
    Serial.write(START_FLAG | REPORT);
    Serial.print("Error: Didn't receive verification on  ");
    Serial.print(writing);
    Serial.write(START_FLAG + STOP_FLAG);
    writing = 0;
  }
  

  if(!writing){
    for(int i=0; i<5; i++){
      if(*(schedules[i])){
        callbacks[i]();
        *(schedules[i]) = false;
        writing = i+1;
        writeTimer = millis() + 100;
        break;
      }
    }
  }

}
