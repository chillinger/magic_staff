#include <Arduino.h>
#include <Wire.h>
#include "ADXL345.h"

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
ADXL345 adxl;

Thread animationThread = Thread();
float global_hue = 2.0;


float exp_mov_avg = 0.0, filter_weight = 1.0/8.0;
float x_mov_avg = 0.0, y_mov_avg=0.0, z_mov_avg=0.0, axis_filter_weight=1/16.0;
float offset_x = (-13.25+12.1)/2.0;
float offset_y = (-17.5+7.25)/2.0;
float offset_z = (15.9+35.7)/2.0;
int ran_cycles = 0;
int cycles_stable_button = 0;
int last_b1=1, last_b2=1, last_b3=1;
bool flashlight_on = false;


ScanAnimation * scanAnimation;
FlashHeadAnimation * flashHeadAnimation;
ShootAnimation * shootAnimation;
BreatheAnimation * breatheAnimation;
StarAnimation * starAnimation;
ShootFromGripAnimation * shootFromGripAnimation;
FlashLightAnimation * flashLightAnimation;
Animation * idleAnimation;

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

void setup_accelerometer(){
  adxl.powerOn();
  //look of tap movement on this axes - 1 == on; 0 == off
  //Y-Axis of accelerometer is oriented along the length of the staff. It's the only axis I'm interested in
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(1);
  adxl.setTapDetectionOnZ(0);
  //set values for what is a tap, and what is a double tap (0-255)
  //set rather high to only react on stomping of the staff
  adxl.setTapThreshold(255); //62.5mg per increment
  adxl.setTapDuration(15); //625μs per increment

  //we need to set up interrupts to a pin, even though we won't connect it - reading the registers is sufficient
  //setting all interupts to take place on int pin 1
  adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN );
  //register interupt actions - 1 == on; 0 == off  
  adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
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
  setup_accelerometer();
  
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
  
  breatheAnimation = new BreatheAnimation(global_hue);
  starAnimation = new StarAnimation(global_hue, 15);
  
  idleAnimation = starAnimation;
  addAnimation(idleAnimation);
  shootFromGripAnimation = new ShootFromGripAnimation(global_hue);
  addAnimation(shootFromGripAnimation);

  flashLightAnimation = new FlashLightAnimation(1.0);
  addAnimation(flashLightAnimation);
  
  animationThread.onRun(animate);
  animationThread.setInterval(3);


  
  
}



void loop(void) 
{

  int b1 = digitalRead(BUTTON1_PIN);
  int b2 = digitalRead(BUTTON2_PIN);
  int b3 = digitalRead(BUTTON3_PIN);

/* Get a new sensor event */     


  /*accel3d measurement = accel3d(event.acceleration.x - offset_x, event.acceleration.y - offset_y, event.acceleration.z - offset_z);   
  float magn = measurement.magnitude();   
  exp_mov_avg = exp_mov_avg + filter_weight * (magn - exp_mov_avg);   
  x_mov_avg = x_mov_avg + axis_filter_weight * (measurement.x - x_mov_avg);   
  y_mov_avg = y_mov_avg + axis_filter_weight * (measurement.y - y_mov_avg);   
  z_mov_avg = z_mov_avg + axis_filter_weight * (measurement.z - z_mov_avg);

  global_hue = 3.0 + 1.0 * measurement.x / magn + 2.0 * measurement.y / magn + 3.0 * measurement.z / magn;*/
  int x,y,z;  
  adxl.readAccel(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  byte interrupts = adxl.getInterruptSource();

    if(animationThread.shouldRun()){
      animationThread.run();
    } 
    
    //tap
    if(adxl.triggered(interrupts, ADXL345_SINGLE_TAP)){
      shootFromGripAnimation->speed = 15.0;
      shootFromGripAnimation->reset();
    } 
    if(last_b1 == 0 && b1 == 1){
      flashlight_on = !flashlight_on;
      if(flashlight_on){
        flashLightAnimation->finished = false;      
      }else{
        flashLightAnimation->finished = true;
        flashLightAnimation->reset();
      }
    }
    if(last_b2 == 0 && b2 == 1){
     if(idleAnimation == starAnimation){
      idleAnimation = breatheAnimation;
     }else{
      idleAnimation = starAnimation;
     }
    }
    
    if(last_b3 == 0 && b3 == 1){
      shootAnimation->speed = 15.0;
      shootAnimation->reset();
    }


    bool all_animations_done = scanAnimation->finished && flashHeadAnimation->finished && shootAnimation->finished && shootFromGripAnimation->finished;
    starAnimation->finished = !all_animations_done;
    if(idleAnimation->finished){
      idleAnimation->reset();
    }
    
   
    last_b1 = b1;
    last_b2 = b2;
    last_b3 = b3;

}
