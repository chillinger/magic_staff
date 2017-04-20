#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_NeoPixel.h>
#include <RingBuf.h>
#include <Thread.h>

#define strip_obenENGTH 62
#define INITIAL_DELAY 10

#define BUTTON1_PIN 7
#define BUTTON2_PIN 8
#define BUTTON3_PIN 9

#define LEDS_OBEN_PIN 2
#define LEDS_UNTEN_PIN 3

#define LEDS_UNTEN 58
#define LEDS_OBEN 24
#define LEDS_SPITZE 7 
/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_NeoPixel strip_unten = Adafruit_NeoPixel(LEDS_UNTEN, LEDS_UNTEN_PIN);
Adafruit_NeoPixel strip_oben = Adafruit_NeoPixel(LEDS_OBEN + LEDS_SPITZE, LEDS_OBEN_PIN);
Thread animationThread = Thread();
float global_hue = 0;


float exp_mov_avg = 0.0, filter_weight = 1.0/8.0;
float x_mov_avg = 0.0, y_mov_avg=0.0, z_mov_avg=0.0, axis_filter_weight=1/16.0;
float offset_x = (-13.25+12.1)/2.0;
float offset_y = (-17.5+7.25)/2.0;
float offset_z = (15.9+35.7)/2.0;
int ran_cycles = 0;
int cycles_stable_button = 0;
enum Axis {X,Y,Z};
class accel3d
{
  public:
  accel3d(float x, float y, float z){
    this->x = x;
    this->y = y;
    this->z = z;
  }

  

  float magnitude(){
    return sqrt(x*x + y*y + z*z);
  }

  Axis main_component(){
    float max_value = max(max(x,y),z);
    if(max_value == x) return Axis::X;
    if(max_value == y) return Axis::Y;
    return Axis::Z;
  }

  float main_component_value(){
    float max_value = max(max(x,y),z);
    if(max_value == x) return x;
    if(max_value == y) return y;
    return z;
  }
  
  float x;
  float y;
  float z;  

};

void led_blank(){
  for(int i = 0; i<LEDS_OBEN; i++){
    
    strip_oben.setPixelColor(i, 0, 0, 0);
  }  
  for(int i = 0; i<LEDS_UNTEN; i++){    
    strip_unten.setPixelColor(i, 0, 0, 0);
  }  

}

void led_show_both(){
  strip_unten.show();
  strip_oben.show();
}

long HSV_to_RGB( float h, float s, float v ) {
  /* modified from Alvy Ray Smith's site: http://www.alvyray.com/Papers/hsv2rgb.htm */
  // H is given on [0, 6]. S and V are given on [0, 1].
  // RGB is returned as a 24-bit long #rrggbb
  int i;
  float m, n, f;

  // not very elegant way of dealing with out of range: return black
  if ((s<0.0) || (s>1.0) || (v<0.0) || (v>1.0)) {
    return 0L;
  }

  if ((h < 0.0) || (h > 6.0)) {
    return long( v * 255 ) + long( v * 255 ) * 256 + long( v * 255 ) * 65536;
  }
  i = floor(h);
  f = h - i;
  if ( !(i&1) ) {
    f = 1 - f; // if i is even
  }
  m = v * (1 - s);
  n = v * (1 - s * f);
  switch (i) {
  case 6:
  case 0:
    return long(v * 255 ) * 65536 + long( n * 255 ) * 256 + long( m * 255);
  case 1:
    return long(n * 255 ) * 65536 + long( v * 255 ) * 256 + long( m * 255);
  case 2:
    return long(m * 255 ) * 65536 + long( v * 255 ) * 256 + long( n * 255);
  case 3:
    return long(m * 255 ) * 65536 + long( n * 255 ) * 256 + long( v * 255);
  case 4:
    return long(n * 255 ) * 65536 + long( m * 255 ) * 256 + long( v * 255);
  case 5:
    return long(v * 255 ) * 65536 + long( m * 255 ) * 256 + long( n * 255);
  }
}

void led_flash_end(){
  for(int i = 0; i<25; i++){
    led_set_both(60, 0,0,i*10+4);
    led_set_both(61, 0,0,i*10+4);
    delayMicroseconds(500);
    led_show_both();
  }
  for(int i = 0; i<25; i++){
    led_set_both(60, 0,0,254-i*10+4);
    led_set_both(61, 0,0,254-i*10+4);
    delayMicroseconds(1500);
    led_show_both();
  }
  led_blank();
  led_show_both();
}

void led_set_both(int pixel, int r, int g , int b){
  strip_unten.setPixelColor(pixel, r, g, b);
  strip_oben.setPixelColor(pixel, r, g, b);
}

void led_set_both(int pixel, uint32_t c){
  strip_unten.setPixelColor(pixel, c);
  strip_oben.setPixelColor(pixel, c);
}

void led_set_head(uint32_t c){
  for(int i=LEDS_OBEN; i<LEDS_OBEN + LEDS_SPITZE; i++){
    strip_oben.setPixelColor(i, c);
  }
}

void led_pulse(int magnitude){
  led_blank();
  for(int i = 0; i<strip_obenENGTH; i++){
    led_blank();
    led_set_both(i,0,0,min(magnitude, 255));   
    led_show_both();
    
    delayMicroseconds(500);
  }
  led_blank();
  led_show_both();
}
class Animation
{
  public:

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
  bool finished = false;
  int brightness = 0;
  uint32_t c = Adafruit_NeoPixel::Color(0,0,255);
  public:
  ScanAnimation(int brightness, Axis axis){
    if (axis == Axis::X)
      this->c = Adafruit_NeoPixel::Color(min(brightness, 255),0,0);
    else if (axis == Axis::Y)
      this->c = Adafruit_NeoPixel::Color(0,min(brightness, 255),0);
    else
      this->c = Adafruit_NeoPixel::Color(0,0,min(brightness, 255));
  }
  ScanAnimation(uint32_t c){
    this->c = c;
  }
  void step(){
    if(!finished){
      if(started == false){
        led_blank();
        started = true;
      }
      if(x < LEDS_UNTEN){
        led_blank();
        float fledo = (float)LEDS_OBEN;
        int l1 = (int)x;
        int l2 = l1 +1;
        float part_l2 = x - (float)l1;
        
        float part_l1 = 1 - part_l2;

        part_l2 *= x/(float)LEDS_UNTEN;
        part_l1 *= x/(float)LEDS_UNTEN;

        if(l2 < LEDS_OBEN){
          led_set_both(l1,HSV_to_RGB(global_hue, 1.0, part_l1)); 
          led_set_both(l2,HSV_to_RGB(global_hue, 1.0, part_l2)); 
        }else{
          strip_unten.setPixelColor(l1,HSV_to_RGB(global_hue, 1.0, part_l1)); 
          strip_unten.setPixelColor(l2,HSV_to_RGB(global_hue, 1.0, part_l2)); 
        }
        currentLED++;
        x += dx;
      }else{
        finished = true;
        currentLED = 0;
        led_blank();
      }
      led_show_both();
    }   
  }
  void reset(){
    x = 0;
    currentLED = 0;
    started = false;
    finished = false;
    led_blank();
    led_show_both();
  }
};

class FlashHeadAnimation: public Animation
{
  
  bool started = false;
  bool finished = false;
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
 double dx = 0.03;
  FlashHeadAnimation(float hue){
    reset();
    this->hue = hue;
  }
  void step(){
    if(!finished){
      if(up){
        value = pow(base, x) - 1;
        x += dx;
        
        if(value >= 1.0f){
          value = 1.0f;
          up = false;
        }
      }else{
        value += down_speed;
        if(value <= 0.0f){
          value = 0.0f;
          finished = true;
        }          
      }
      uint32_t c = HSV_to_RGB(hue, saturation, value);
      led_set_head(c);     
      strip_oben.show();
    }   
  }

  void reset(){
      started = false;
     finished = false;
     up = true;
     up_speed = 0.01f;
     down_speed = -0.03f;
     hue = global_hue;
     saturation = 1.0;
     value = 0.0f;
     x = 0;
  }
};

Animation * currentAnimation = new Animation();
Animation * scanAnimation;
FlashHeadAnimation * flashHeadAnimation;


void animate(){
  currentAnimation->step();
  
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
  scanAnimation = new ScanAnimation(255, Axis::X);
  flashHeadAnimation = new FlashHeadAnimation(Adafruit_NeoPixel::Color(0,0,255));

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
      scanAnimation->reset();
      currentAnimation = scanAnimation;
    }
    if(b2 == 0){
      flashHeadAnimation->dx = 0.2;
      flashHeadAnimation->reset();
      currentAnimation = flashHeadAnimation;
    }
      strip_unten.setPixelColor(0, HSV_to_RGB(global_hue, 1.0, 0.2));
  strip_unten.show();
    
    delay(1);
    

}
