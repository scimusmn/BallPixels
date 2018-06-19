#include "arduino.h"

class TimeOut {
public:
  bool running;
  unsigned long timer;
  void (*callback)();

  TimeOut(){
    timer = 0;
    running = false;
  }

  void set(void (*CB)(), unsigned long time){
    running = true;
    timer = millis() + time;
    callback = CB;
  }
  void clear(){
    running = false;
  }
  
  void idle(){
    if(timer < millis() && running){
      running = false;
      callback();
    }
  }
};

TimeOut timeOuts[50];

int setTimeout(void (*CB)(), unsigned long time){
  int ret = -1;
  if(time == 0) CB();
  else for(int i =0; i< 50; i++){
    if(timeOuts[i].running == false){
      timeOuts[i].set(CB,time); 
      ret = i;
      break;
    }
  }

  return ret;
}

void idleTimers(){
  for(int i =0; i< 50; i++){
    timeOuts[i].idle();
  }
}

void clearTimeout(int &which){
  if(which >=0) timeOuts[which].running = false;
  which = -1;
}
