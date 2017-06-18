#include "ledTools.h"


Adafruit_NeoPixel strip_unten = Adafruit_NeoPixel(LEDS_UNTEN, LEDS_UNTEN_PIN);
Adafruit_NeoPixel strip_oben = Adafruit_NeoPixel(LEDS_OBEN + LEDS_SPITZE, LEDS_OBEN_PIN);

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
  long ret = 0;
  switch (i) {    
    case 6:
    case 0:
      ret = long(v * 255 ) * 65536 + long( n * 255 ) * 256 + long( m * 255);
      break;
    case 1:
      ret = long(n * 255 ) * 65536 + long( v * 255 ) * 256 + long( m * 255);
      break;
    case 2:
      ret = long(m * 255 ) * 65536 + long( v * 255 ) * 256 + long( n * 255);
      break;
    case 3:
      ret = long(m * 255 ) * 65536 + long( n * 255 ) * 256 + long( v * 255);
      break;
    case 4:
      ret = long(n * 255 ) * 65536 + long( m * 255 ) * 256 + long( v * 255);
      break;
    case 5:
      ret = long(v * 255 ) * 65536 + long( m * 255 ) * 256 + long( n * 255);
      break;
  }
  return ret;
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

//set all leds on staff except head to one color
void led_set_staff(uint32_t c){
  for(int i=0; i<LEDS_OBEN + LEDS_UNTEN; i++){
    set_pixel_on_staff(i, c);
  }
}


float total_leds = (float)LEDS_UNTEN + (float)LEDS_OBEN;
float distance_between_pixels = STAFF_CM/total_leds;
float light_distance = 5.0;
float current_pixel_pos = 0.0;

//Punkt auf dem Stab in cm von unten
float point_on_staff = 0.0;

/**
 * Berechnet die Nähe des gegebenen Pixels zum aktuellen Punkt auf dem Stab
 * je näher, desto höher
 * 
 * 0 ist die untere Spitze, STAFF_CM die obere
 * Pixel 0 ist das unterste Pixel, LEDS_UNTEN + LEDS_OBEN das oberste
 */
float brightness_of_pixel(float position, int pixel, float radius){
  float current_pixel_pos = position / distance_between_pixels;
  float distance_pixels =  abs(current_pixel_pos - pixel);
  float brightness = 0;

  brightness = max(0.0, radius - distance_pixels) / radius ;
  return brightness;
}
void set_pixel_on_staff(int pixel, uint32_t color){
  if(pixel < LEDS_UNTEN){
    strip_unten.setPixelColor(LEDS_UNTEN - pixel, color);
  }else{
    strip_oben.setPixelColor(pixel - LEDS_UNTEN , color);
  }
}
