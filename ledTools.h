#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifndef LEDTOOLS_H
#define LEDTOOLS_H

#define LEDS_UNTEN 58
#define LEDS_OBEN 24
#define LEDS_SPITZE 7 

#define LEDS_OBEN_PIN 2
#define LEDS_UNTEN_PIN 3


#define STAFF_CM  170.0
extern float total_leds;
extern float distance_between_pixels;
extern float light_distance;
extern float current_pixel_pos;

//Punkt auf dem Stab in cm von unten
extern float point_on_staff;
void set_pixel_on_staff(int pixel, uint32_t color);

/**
 * Berechnet die Nähe des gegebenen Pixels zum aktuellen Punkt auf dem Stab
 * je näher, desto höher
 * 
 * 0 ist die untere Spitze, STAFF_CM die obere
 * Pixel 0 ist das unterste Pixel, LEDS_UNTEN + LEDS_OBEN das oberste
 */
float brightness_of_pixel(float position, int pixel, float radius);

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
