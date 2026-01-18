//#ifndef movement_h
//#define movement_h
#pragma once
//init
void init_motors();

//manual / absolut
void move(float xmm, float zmm);

//manual independent pads
void move_external_vertical_right (float mm);
void move_external_vertical_left (float mm);
void move_internal_vertical_right (float mm);
void move_internal_vertical_left (float mm);
void move_horizontal_left(float mm);
void move_horizontal_right(float mm);


//homing
void GoHomePair(float& posX, float& posZ);
void SecondTouchPair(long speed_us);
void BackoffAll(int steps, long speed_us);

//backlash
void adjustmentZ();
void antiBacklashZ(int cycles, int steps, long speed_us);

//#endif