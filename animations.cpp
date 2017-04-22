#import "animations.h"



  
  ScanAnimation::ScanAnimation(int hue){
    this->hue = hue;
  }
  void ScanAnimation::step(){
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
          led_set_both(l1,HSV_to_RGB(hue, 1.0, part_l1)); 
          led_set_both(l2,HSV_to_RGB(hue, 1.0, part_l2)); 
        }else{
          strip_unten.setPixelColor(l1,HSV_to_RGB(hue, 1.0, part_l1)); 
          strip_unten.setPixelColor(l2,HSV_to_RGB(hue, 1.0, part_l2)); 
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
  void ScanAnimation::reset(){
    x = 0;
    currentLED = 0;
    started = false;
    finished = false;
    led_blank();
    led_show_both();
  }

  FlashHeadAnimation::FlashHeadAnimation(float hue){
    reset();
    this->hue = hue;
  }
  void FlashHeadAnimation::step(){
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

  void FlashHeadAnimation::reset(){
     started = false;
     finished = false;
     up = true;
     up_speed = 0.01f;
     down_speed = -0.03f;     
     saturation = 1.0;
     value = 0.0f;
     x = 0;
  }


