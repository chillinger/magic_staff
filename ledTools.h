#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifndef LEDTOOLS_H
#define LEDTOOLS_H

#define LEDS_UNTEN 58
#define LEDS_OBEN 24
#define LEDS_SPITZE 7 

#define LEDS_OBEN_PIN 2
#define LEDS_UNTEN_PIN 3

extern Adafruit_NeoPixel strip_unten;
extern Adafruit_NeoPixel strip_oben;
void led_blank();

void led_show_both();
long HSV_to_RGB( float, float , float  );

void led_flash_end();
void led_set_both(int , int , int  , int );

void led_set_both(int , uint32_t );
void led_set_head(uint32_t );

#endif
