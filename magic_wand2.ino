#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include <RingBuf.h>
#include <Thread.h>
#include "accelTools.h"
#include "animations.h"


#define INITIAL_DELAY 10

#define BUTTON1_PIN 7
#define BUTTON2_PIN 8
#define BUTTON3_PIN 9



#include "ledTools.h"


/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

Thread animationThread = Thread();
float global_hue = 2.0;


float exp_mov_avg = 0.0, filter_weight = 1.0/8.0;
float x_mov_avg = 0.0, y_mov_avg=0.0, z_mov_avg=0.0, axis_filter_weight=1/16.0;
float offset_x = (-13.25+12.1)/2.0;
float offset_y = (-17.5+7.25)/2.0;
float offset_z = (15.9+35.7)/2.0;
int ran_cycles = 0;
int cycles_stable_button = 0;

void led_pulse(int magnitude){
  led_blank();
  for(int i = 0; i<LEDS_UNTEN; i++){
    led_blank();
    led_set_both(i,0,0,min(magnitude, 255));   
    led_show_both();
    
    delayMicroseconds(500);
  }
  led_blank();
  led_show_both();
}


ScanAnimation * scanAnimation;
FlashHeadAnimation * flashHeadAnimation;
ShootAnimation * shootAnimation;
BreatheAnimation * breatheAnimation;
StarAnimation * starAnimation;
ShootFromGripAnimation * shootFromGripAnimation;


int max_parallel_animations = 10;
Animation ** animations = malloc(sizeof(Animation*) * max_parallel_animations);
int num_animations = 0;

void addAnimation(Animation * a){

  if(num_animations < max_parallel_animations){
      animations[num_animations++] = a;
  }

}

void clearAnimations(){
  animations = malloc(sizeof(Animation*) * max_parallel_animations);
  num_animations = 0;
}

void animate(){

  for(int i=0; i<num_animations; i++){
    animations[i]->step();     
  }
  led_show_both();
  
}



void setup(void) 
{
#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Accelerometer Test"); Serial.println("");
  if(!accel.begin())
  {
     /* There was a problem detecting the ADXL345 ... check your connections */
      while(1);
  }
  
  strip_unten.begin();
  strip_unten.show();
  strip_oben.begin();
  strip_oben.show();
  scanAnimation = new ScanAnimation(global_hue);
  addAnimation(scanAnimation);
  flashHeadAnimation = new FlashHeadAnimation(Adafruit_NeoPixel::Color(0,0,255));
  addAnimation(flashHeadAnimation);
  shootAnimation = new ShootAnimation(global_hue);
  addAnimation(shootAnimation);
  //breatheAnimation = new BreatheAnimation(global_hue);
  //addAnimation(breatheAnimation);
  starAnimation = new StarAnimation(global_hue, 15);
  addAnimation(starAnimation);
  shootFromGripAnimation = new ShootFromGripAnimation(global_hue);
  addAnimation(shootFromGripAnimation);
  
  animationThread.onRun(animate);
  animationThread.setInterval(3);


  
  
}


void loop(void) 
{

  int b1 = digitalRead(BUTTON1_PIN);
  int b2 = digitalRead(BUTTON2_PIN);
  int b3 = digitalRead(BUTTON3_PIN);

/* Get a new sensor event */     
  sensors_event_t event;    
  accel.getEvent(&event);   
  
  accel3d measurement = accel3d(event.acceleration.x - offset_x, event.acceleration.y - offset_y, event.acceleration.z - offset_z);   
  float magn = measurement.magnitude();   
  exp_mov_avg = exp_mov_avg + filter_weight * (magn - exp_mov_avg);   
  x_mov_avg = x_mov_avg + axis_filter_weight * (measurement.x - x_mov_avg);   
  y_mov_avg = y_mov_avg + axis_filter_weight * (measurement.y - y_mov_avg);   
  z_mov_avg = z_mov_avg + axis_filter_weight * (measurement.z - z_mov_avg);

  global_hue = 3.0 + 1.0 * measurement.x / magn + 2.0 * measurement.y / magn + 3.0 * measurement.z / magn;
  /*Serial.print("hue: ");
  Serial.print(global_hue);
  Serial.print(", x norm: ");
  Serial.print(1.0 * measurement.x / magn);
  Serial.print(", y norm: ");
  Serial.print(2.0 * measurement.y / magn);
  Serial.print(", z norm: ");
  Serial.println(3.0 * measurement.z / magn);*/


  //if(ran_cycles > INITIAL_DELAY){
    if(animationThread.shouldRun()){
      animationThread.run();
    } 
    

    if(b1 == 0){
      shootFromGripAnimation->speed = 15.0;
      shootFromGripAnimation->reset();
      
    }
    if(b2 == 0){
      
      flashHeadAnimation->reset();
    }
    if(b3 == 0){
      shootAnimation->speed = 15.0;
      shootAnimation->reset();
    }

    bool all_animations_done = scanAnimation->finished && flashHeadAnimation->finished && shootAnimation->finished && shootFromGripAnimation->finished;
    starAnimation->finished = !all_animations_done;
    if(starAnimation->finished){
      starAnimation->reset();
    }
   
    

}
