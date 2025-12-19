#ifndef movement_h
#define movement_h

void init_motors();
void move(float xmm, float zmm);
void move_external_vertical_right (float mm);
void move_external_vertical_left (float mm);
void move_internal_vertical_right (float mm);
void move_internal_vertical_left (float mm);
void move_horizontal_left(float mm);
void move_horizontal_right(float mm);



#endif