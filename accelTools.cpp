#include "accelTools.h"
#include <Arduino.h>

accel3d::accel3d(float x, float y, float z){
  this->x = x;
  this->y = y;
  this->z = z;
}

float accel3d::magnitude(){
  return sqrt(x*x + y*y + z*z);
}

Axis accel3d::main_component(){
  float max_value = max(max(x,y),z);
  if(max_value == x) return Axis::X;
  if(max_value == y) return Axis::Y;
  return Axis::Z;
}

float accel3d::main_component_value(){
  float max_value = max(max(x,y),z);
  if(max_value == x) return x;
  if(max_value == y) return y;
  return z;
}
  
