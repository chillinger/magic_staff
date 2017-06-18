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

    }   
  }
  void ScanAnimation::reset(){
    x = 0;
    currentLED = 0;
    started = false;
    finished = false;
  }

  FlashHeadAnimation::FlashHeadAnimation(float hue){
    reset();
    finished = true;
    this->hue = hue;
  }
  FlashHeadAnimation::FlashHeadAnimation(float hue, float up_speed, float down_speed){
    reset();
    finished = true;
    this->hue = hue;
    this->up_speed = up_speed;
    this->down_speed = down_speed;
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

ShootAnimation::ShootAnimation(float hue){
  flashHeadAnimation = new FlashHeadAnimation(hue);
  this->speed = 7.0;
  this->hue = hue;
  current_position = 0.0;
}
ShootAnimation::ShootAnimation(float hue, float speed){
  flashHeadAnimation = new FlashHeadAnimation(hue, 0.02, 0.001);
  this->speed = speed;
  this->hue = hue;
  current_position = 1.0;
}
void ShootAnimation::step(){
  if(!finished){
    if(!staff_done){
      led_blank();
      if(this->current_position > STAFF_CM + 10.0){
        staff_done = true;
      }
      for(int i = 0; i< total_leds; i++){
        set_pixel_on_staff(i, HSV_to_RGB(hue, 1.0, brightness_of_pixel(this->current_position, i, current_position/20.0 + 3)));
      }
      this->current_position += this->speed;
    }else{
      led_blank();
      if(!flashHeadAnimation->finished){
        flashHeadAnimation->step();
      }else{
        finished = true;
      }
    }
    
  }
}

void ShootAnimation::reset(){
  current_position = 1.0;
  finished = false;
  staff_done = false;
  flashHeadAnimation->reset();
}


BreatheAnimation::BreatheAnimation(float hue){
  this->hue = hue;
  reset();
}
void BreatheAnimation::step(){
  if(!finished){
    if(current_value > max_value){
      current_value = max_value;
      speed = speed * -1;
    }
    if(current_value < min_value){
      current_value = min_value;
      speed = speed * -1;
    }
    current_value += speed;
    for(int i = 0; i< total_leds; i++){
      set_pixel_on_staff(i, HSV_to_RGB(hue, 1.0, current_value));
    }
  }
}
void BreatheAnimation::reset(){
  current_value = min_value;
  finished = false;
}

StarAnimation::StarAnimation(float hue, int num_stars){
  this->hue = hue;
  this->num_stars = num_stars;
  this->stars = malloc(sizeof(int) * num_stars);
  this->star_value = malloc(sizeof(float) * num_stars);
  this->star_speed = malloc(sizeof(float) * num_stars);
  this->star_hue = malloc(sizeof(float) * num_stars);
  reset();
}
void StarAnimation::step(){
  if(!finished){
    led_blank();
    for(int i = 0; i< num_stars; i++){

      float * current_value = star_value + i;
      float * current_speed = star_speed + i;
      if(*current_value > max_value){
        *current_value = max_value;
        *current_speed = *current_speed * -1;
      }
      if(*current_value < min_value){
        stars[i] = random(0, total_leds);
        star_hue[i] = (float)random(0, 600)/100.0;
        
        *current_value = min_value;
        *current_speed = *current_speed * -1;
      }
      *current_value += *current_speed;
      set_pixel_on_staff(stars[i], HSV_to_RGB(star_hue[i], 1.0, star_value[i]));
    }
  }
}
void StarAnimation::reset(){
  for(int i = 0; i< num_stars; i++){
    stars[i] = random(0, total_leds);
    star_value[i] = random(min_value*1000, max_value*1000)/1000.0;
    star_hue[i] = (float)random(0, 600)/100.0;
    star_speed[i] = random(0, 1) % 2 ? speed : speed * -1;
  }
  led_blank();
}



ShootFromGripAnimation::ShootFromGripAnimation(float hue){
  flashHeadAnimation = new FlashHeadAnimation(hue);
  this->speed = 7.0;
  this->hue = hue;
  current_position_up = 120.0;
  current_position_down=current_position_up;
}
ShootFromGripAnimation::ShootFromGripAnimation(float hue, float speed){
  flashHeadAnimation = new FlashHeadAnimation(hue, 0.02, 0.001);
  this->speed = speed;
  this->hue = hue;
  current_position_up = 120.0;
  current_position_down=current_position_up;
}
void ShootFromGripAnimation::step(){
  if(!finished){
    led_blank();
    if(this->current_position_up > STAFF_CM + 10.0){
      staff_up_done = true;
    }
    if(this->current_position_down < -10.0){
      staff_down_done = true;
    }
    for(int i = 0; i< total_leds; i++){
      float brightness = brightness_of_pixel(this->current_position_up, i, current_position_up/20.0 + 3) + brightness_of_pixel(this->current_position_down, i, current_position_down/20.0 + 3);
      set_pixel_on_staff(i, HSV_to_RGB(hue, 1.0, min(1.0,brightness)));
      
    }
    this->current_position_up += this->speed;
    this->current_position_down -= this->speed;
    if(staff_up_done){
      if(!flashHeadAnimation->finished){
        flashHeadAnimation->step();
      }else{
        finished = true;
      }
    }
    
  }
}

void ShootFromGripAnimation::reset(){
  current_position_up = 120.0;
  current_position_down=current_position_up;
  finished = false;
  staff_up_done = false;
  staff_down_done = false;
  flashHeadAnimation->reset();
}


//============== FLASHLIGHT
FlashLightAnimation::FlashLightAnimation(float value){
  this->value = value;
}
void FlashLightAnimation::step(){
  if(!finished){
    led_set_head(HSV_to_RGB(0, 0.0, value));
  }
}

void FlashLightAnimation::reset(){
  led_set_head(HSV_to_RGB(0, 0.0, 0.0));
}

