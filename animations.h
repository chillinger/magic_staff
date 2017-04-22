#include "accelTools.h"
#include "ledTools.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifndef ANIMATIONS_H
#define ANIMATIONS_H



class Animation
{
  public:
  bool finished = false;
  virtual void step(){
    //nop
  }
  virtual void reset(){
    //nop
  }
};

class ScanAnimation: public Animation
{
  int currentLED = 0;
  float x = 0;
  float dx = 0.1;
  bool started = false;
  int brightness = 0;
  int hue;
  uint32_t c = Adafruit_NeoPixel::Color(0,0,255);
  public:

  ScanAnimation(int);
  void step();
  void reset();
};

class FlashHeadAnimation: public Animation
{
      
  bool started = false;
  bool up = true;
  float up_speed = 0.005;
  float down_speed = 0.01;
  float hue = 0.0;
  float saturation = 1.0;
  float value = 0.0;
  double x = 0;
  
  double ticks = 0;
  double base = 1.5;
  int steps = 0;
  public:
  double dx;
  FlashHeadAnimation(float hue);
  void step();
  void reset();
};

#endif
