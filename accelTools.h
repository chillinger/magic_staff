

#ifndef ACCELTOOLS_H
#define ACCELTOOLS_H
enum Axis {X,Y,Z};
class accel3d
{
  public:
  accel3d(float x, float y, float z);
  float magnitude();
  Axis main_component();

  float main_component_value();
  
  float x;
  float y;
  float z;  

};

#endif
