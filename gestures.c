#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "ui.h"
#include "gestures.h"
#include "keyinjector.h"
#include "gesture_definition.h"
#include "eventreader.h"
#include "config.h"

static int keys_down = 0;
static int segment_count = 0;

struct Segment { 
    struct Point start;
    struct Point end;
};

static struct Segment segments[MAX_SLOTS+1];

void recognize_gestures(struct TouchEvent *f) {
    int slot = f->slot;
    if (f->status == Down){
        debug_print("slot %d down x:%d, y:%d  \n", slot, f->x, f->y);

        if(keys_down < 0) keys_down = 0; //todo: fixit
        segments[slot].start.x = f->x;
        segments[slot].start.y = f->y;
        segments[slot].start.time = f->time;

        keys_down++;
        segment_count++;
    }
    else if (f->status == Up){
        debug_print("slot %d up x:%d, y:%d segments:%d \n", slot, f->x, f->y, segment_count);
        int x = f->x;
        int y = f->y;

        keys_down--;
        segments[slot].end.x = x;
        segments[slot].end.y = y;
        segments[slot].end.time = f->time;

        //todo: extract gesture recognition

        if(keys_down == 0){
            struct Gesture gesture;
            struct Segment *p = segments;
            int dx,dy,distance=0;
			unsigned long dt;
            switch(segment_count){

                //single tap
                case 1: 
                    printf("Tap  x:%d, y:%d  raw_x:%d, raw_y:%d\n", f->x, f->y, f->raw_position.x, f->raw_position.y);
                    dx = p->end.x - p->start.x;
                    dy = p->end.y - p->start.y;
					dt = p->end.time - p->start.time;
					printf("delta dt %lu\n", dt);
					distance = (int)sqrt(dx*dx+dy*dy);

                    if (distance < JITTER) {

                        int nav_stripe = SCREEN_WIDTH /3;
                        if (y > 100 && x > 100){//disable upper stripe and left menus
                            if (x < nav_stripe) { 
                                if (y > SCREEN_HEIGHT - 150 && x < 150) {
                                    printf("TOC\n");
                                }
                                else {
                                    gesture.type = TapLeft; 
                                    interpret_gesture(&gesture);
                                }
                            }
                            else if (x > nav_stripe*2) {
                                    gesture.type = TapRight; 
                                    interpret_gesture(&gesture);
                            }
                        }
                    }
                    else {
						unsigned int velo = (10*distance) / (dt == 0 ? 1 : dt);
						printf("velocity %d\n", velo);
						if (velo < SWIPE_VELOCITY) //ignore slow swipes
							break;
                        //swipe 
                        if (abs(dx) > abs(dy)) {
                            //horizontal
                            if (dx < 0 && dx < -500 && abs(dy) < 150) {
                                printf("swipe left\n");
								gesture.type = SwipeLeft; 
								interpret_gesture(&gesture);
                            }
                            else if (dx > 0 && dx > 500 && abs(dy) < 150) {
                                printf("swipe right\n");
                                gesture.type = SwipeRight; 
                                interpret_gesture(&gesture);
                            }
                        }
                        else {
                            //vertical
                            if (dy > 0 && dy > 600) {
                                gesture.type = SwipeDownLong; 
                                interpret_gesture(&gesture);
                            }
                            else if (dy < 0 && dy < -600) {
                                gesture.type = SwipeUpLong;
                                interpret_gesture(&gesture);
                            }
                        }
                    }
                    break;
                /*
                // DISABLE THE FUNCTIONALITY TO ENABLE/DISABLE
                case 7:
                    dx = segments[0].end.x - segments[1].end.x;
                    dy = segments[0].end.y - segments[1].end.y;
                    distance = (int)sqrt(dx*dx+dy*dy);
                    if (distance > TWOTAP_DISTANCE) {
                        gesture.type = TwoTapWide;
                        interpret_gesture(&gesture);
                    }
                    gesture.type = TwoTapWide;
                    interpret_gesture(&gesture);
                    break;
                */
                default:
                    printf("%d finger tap\n",segment_count);
                    break;
            }
            segment_count=0;
        }
    }
}
