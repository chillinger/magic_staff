#include "accelTools.h"
#include "ledTools.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifndef ANIMATIONS_H
#define ANIMATIONS_H



class Animation
{
  public:
  bool finished = true;
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
  bool started = false;
  int brightness = 0;
  int hue;
  uint32_t c = Adafruit_NeoPixel::Color(0,0,255);
  public:
  double dx = 0.1;
  ScanAnimation(int);
  void step();
  void reset();
};

class FlashHeadAnimation: public Animation
{
      
  bool started = false;
  bool up = true;
  float up_speed = 0.1;
  float down_speed = 0.3;
  float hue = 0.0;
  float saturation = 0.0;
  float value = 0.0;
  double x = 0;
  
  double ticks = 0;
  double base = 1.5;
  int steps = 0;
  public:
  double dx = 1;
  FlashHeadAnimation(float hue);
  FlashHeadAnimation(float hue, float up_soeed, float down_speed);
  void step();
  void reset();
};

class ShootAnimation: public Animation
{
  float current_position;
  bool staff_done = false;
  FlashHeadAnimation * flashHeadAnimation;
  public:
  float speed;
  float hue;
  ShootAnimation(float hue);
  ShootAnimation(float hue, float speed);
  void step();
  void reset();
};

class ShootFromGripAnimation: public Animation
{
  float current_position_up;
  float current_position_down;
  bool staff_up_done = false;
  bool staff_down_done = false;
  FlashHeadAnimation * flashHeadAnimation;
  public:
  float speed;
  float hue;
  ShootFromGripAnimation(float hue);
  ShootFromGripAnimation(float hue, float speed);
  void step();
  void reset();
};

class BreatheAnimation: public Animation
{
  float current_value=0.0;
  public:
  float speed = 0.0005;
  float hue;
  float max_value = 0.05;
  float min_value = 0.001;
  BreatheAnimation(float hue);
  void step();
  void reset();
};

class StarAnimation: public Animation
{
  int * stars;
  float * star_value;
  float * star_speed;
  float * star_hue;
  public:
  float speed = 0.0005;
  float hue;
  float max_value = 0.05;
  float min_value = 0.001;
  int num_stars;
  StarAnimation(float hue, int num_stars);
  void step();
  void reset();
};

class FlashLightAnimation: public Animation
{

  public:
  float hue;
  float value;
  FlashLightAnimation(float value);
  void step();
  void reset();
};
#endif
