#include <AltSoftSerial.h>
#include "serialParser.h"
//#include "timeOut.h"

#define COLUMN_COLOR 64
#define READY 127
#define BALL_DETECT 32
#define NUM_COLUMNS 16

////////// Light strip defines:
#define SET_ADDRESS 3
#define SET_COLOR 2
#define GET_STATES 1

bool * states;
char * counts;

char colors[5][3] = {
  {127, 127, 127},
  {0,127,0},
  {0, 100,100},
  {0, 0, 64},
  {0,0,0}
};

char imgs[4][13][13] = {
  {
    {0,0,0,0,2,2,2,2,2,0,0,0,0},
    {0,0,3,3,3,3,1,3,2,2,2,0,0},
    {0,0,3,1,3,1,1,3,1,1,1,0,0},
    {0,0,3,1,3,3,1,1,3,1,1,1,0},
    {0,0,3,3,1,1,1,3,3,3,3,0,0},
    {0,0,0,0,1,1,1,1,1,1,0,0,0},
    {0,0,3,3,3,2,3,2,3,3,3,0,0},
    {0,3,3,3,3,2,2,2,3,3,3,3,0},
    {0,1,1,3,2,1,2,1,2,3,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,1,1,0},
    {0,0,1,2,2,2,2,2,2,2,0,1,0},
    {0,0,3,3,3,0,0,0,3,3,3,0,0},
    {0,3,3,3,3,0,0,0,3,3,3,3,0},
  },
  {
    {0,1,2,3,0,1,2,3,0,1,2,3,0},
    {1,2,3,0,1,2,3,0,1,2,3,0,1},
    {2,3,0,1,2,3,0,1,2,3,0,1,2},
    {3,0,1,2,3,0,1,2,3,0,1,2,3},
    {0,1,2,3,0,1,2,3,0,1,2,3,0},
    {1,2,3,0,1,2,3,0,1,2,3,0,1},
    {2,3,0,1,2,3,0,1,2,3,0,1,2},
    {3,0,1,2,3,0,1,2,3,0,1,2,3},
    {0,1,2,3,0,1,2,3,0,1,2,3,0},
    {1,2,3,0,1,2,3,0,1,2,3,0,1},
    {2,3,0,1,2,3,0,1,2,3,0,1,2},
    {3,0,1,2,3,0,1,2,3,0,1,2,3},
  },
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
  },
  {
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
  },
};


AltSoftSerial serial;

serialParser cells(serial);

bool scheduleRead = 0, scheduleColor = 0, scheduleCount = 1; 
int writing = 0;

char currentImage = 0;
char colAddr = 1;
char r = 0;
char g = 127;
char b = 0;

int numColumns = 0;

unsigned long writeTimer = 0;
unsigned long countTimer = 0;
char colorCount = 0;
unsigned long setColorTimer = 0;

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(115200);
  serial.begin(19200);

  pinMode(5,INPUT);
  pinMode(6,OUTPUT);
  
  //parser.address = 1;

  cells.address = REPORT;

//  parser.on(READY, [](unsigned char * input, int size){
//    parser.sendPacket(REPORT,READY);
//  });

//  parser.on(COLUMN_COLOR, [](unsigned char * input, int size){
//    colAddr = input[2];
//    r = input[3];
//    g = input[4];
//    b = input[5];
//    scheduleColor = true;
//    //cells.sendPacket(input[2],SET_COLOR, input[3], input[4], input[5]);
//  });

  cells.on(SET_COLOR, [](unsigned char * input, int size){
    writing = 0;
    Serial.println("Got color confirmation");
    Serial.println(setColorTimer - millis(),DEC);
    setColorTimer = millis() - 1;
  });

  cells.on(SET_ADDRESS, [](unsigned char * input, int size){
    numColumns = input[2];
    //parser.sendPacket(REPORT, NUM_COLUMNS, numColumns);
    states = new bool[numColumns];
    counts = new char[numColumns];
    for(int i=0; i<numColumns; i++){
      states[i] = 1;
      counts[i] = 0;
    }
    writing = 0;
  });

  cells.on(GET_STATES, [](unsigned char * input, int size){
    //numColumns = input[2];
    int curBoard = 0;
    bool changed = false;
    for( int j = 0; j < (numColumns / 7) + 1; j++){
      int nex = input[3 + j];
      
      do {
        int curVal = bitRead(nex,curBoard % 7);
        if(curVal != states[curBoard]){
          //parser.sendPacket(REPORT, BALL_DETECT, curBoard);
          if(curVal && counts[curBoard] < 13){
            Serial.print(curBoard);
            Serial.print(" at: ");
            counts[curBoard] = (counts[curBoard] + 1);
            Serial.println(counts[curBoard], DEC);
            colAddr = curBoard + 1;
            scheduleColor = true;
          } else if(counts[curBoard] >= 13){
            scheduleColor = true;
          }
          states[curBoard] = curVal;
        }
                      
        curBoard++;
      } while(curBoard % 7 && curBoard <= numColumns);
    }
    writing = 0;
  });

  //parser.sendPacket(REPORT,READY);
}

void requestCount(){
  cells.sendPacket(BROADCAST, SET_ADDRESS,0);
}

//void setColor(){
//  cells.sendPacket(colAddr,SET_COLOR, r,g,b);
//}

void setColor(){
  int col = 4;
  if(counts[colAddr-1] < 13) col = imgs[currentImage][12-counts[colAddr-1]][colAddr-1];
//  Serial.print(colAddr-1);
//  Serial.print(" : ");
//  Serial.print(counts[colAddr-1], DEC);
//  Serial.print(" being set to ");
//  Serial.print(colors[col][0], DEC);
//  Serial.print(",");
//  Serial.print(colors[col][1], DEC);
//  Serial.print(",");
//  Serial.println(colors[col][2], DEC);
  cells.sendPacket(colAddr,SET_COLOR, colors[col][0],colors[col][1],colors[col][2]);
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

  if(!digitalRead(5)){
    digitalWrite(6,HIGH);
    delay(5000);
    digitalWrite(6,LOW);
  }
  //parser.idle();
  cells.idle();

  if(numColumns && colorCount<numColumns && setColorTimer< millis()){
    setColorTimer = millis() + 100;
    colAddr = colorCount+1;
    scheduleColor = true;
    colorCount++;
  }

  if(countTimer < millis()){
    countTimer = millis() + 200;
    scheduleRead = true;
  }

  if(writeTimer < millis() && writing){
//    Serial.write(START_FLAG | REPORT);
    Serial.print("Error: Didn't receive verification on  ");
    Serial.println(writing);
//    Serial.write(START_FLAG + STOP_FLAG);
    writing = 0;
  }
  

  if(!writing){
    for(int i=0; i<3; i++){
      if(*(schedules[i])){
        //Serial.print("writing: ");
        //Serial.println(i,DEC);
        callbacks[i]();
        *(schedules[i]) = false;
        writing = i+1;
        writeTimer = millis() + 100;
        break;
      }
    }
  } else {
    //Serial.println(writing);
  }

}
