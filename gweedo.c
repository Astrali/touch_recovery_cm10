//START GWEEDO TOUCH CODE
static int in_touch = 0; //1 = in a touch
static int slide_right = 0;
static int slide_left = 0;
static int touch_x = 0;
static int touch_y = 0;
static int old_x = 0;
static int old_y = 0;
static int diff_x = 0;
static int diff_y = 0;

static void reset_gestures() {
    diff_x = 0;
    diff_y = 0;
    old_x = 0;
    old_y = 0;
    touch_x = 0;
    touch_y = 0;
}

static int input_callback(int fd, short revents, void *data)
{
    struct input_event ev;
    int ret;
    int fake_key = 0;
    gr_surface surface = gVirtualKeys;

    ret = ev_get_input(fd, revents, &ev);
    if (ret)
        return -1;

#ifdef BOARD_TOUCH_RECOVERY
    if (touch_handle_input(fd, ev))
      return 0;
#endif

    if (ev.type == EV_SYN) {
        return 0;
    } else if (ev.type == EV_REL) {
        if (ev.code == REL_Y) {
            // accumulate the up or down motion reported by
            // the trackball.  When it exceeds a threshold
            // (positive or negative), fake an up/down
            // key event.
            rel_sum += ev.value;
            if (rel_sum > 3) {
                fake_key = 1;
                ev.type = EV_KEY;
                ev.code = KEY_DOWN;
                ev.value = 1;
                rel_sum = 0;
            } else if (rel_sum < -3) {
                fake_key = 1;
                ev.type = EV_KEY;
                ev.code = KEY_UP;
                ev.value = 1;
                rel_sum = 0;
            }
        }
    } else {
        rel_sum = 0;
    }

#ifdef SK8S_DEBUG_UI
    printf("ev.type: %x, ev.code: %x, ev.value: %i\n", ev.type, ev.code, ev.value);
#endif
//    if(ev.type == EV_ABS && ev.code == ABS_MT_TRACKING_ID) {  //debugging
    if (ev.type == 3 && ev.code == 48 && ev.value != 0) {
        if (in_touch == 0) {
            in_touch = 1; //starting to track touch...
            reset_gestures();
        }
    } else if (ev.type == 3 && ev.code == 48 && ev.value == 0) {
            //finger lifted! lets run with this
            ev.type = EV_KEY; //touch panel support!!!
            int keywidth = gr_get_width(surface) / 4;
            int keyoffset = (gr_fb_width() - gr_get_width(surface)) / 2;
            if (touch_y > (gr_fb_height() - gr_get_height(surface)) && touch_x > 0) {
                //they lifted in the touch panel region
                if (touch_x < (keywidth + keyoffset)) {
                    //down button
                    ev.code = KEY_DOWN;
                    reset_gestures();
                } else if (touch_x < ((keywidth * 2) + keyoffset)) {
                    //up button
                    ev.code = KEY_UP;
                    reset_gestures();
                } else if (touch_x < ((keywidth * 3) + keyoffset)) {
                    //back button
                    ev.code = KEY_BACK;
                    reset_gestures();
                } else {
                    //enter key
                    ev.code = KEY_ENTER;
                    reset_gestures();
                }
            }
            if (slide_right == 1) {
                ev.code = KEY_ENTER;
                slide_right = 0;
            } else if (slide_left == 1) {
                ev.code = KEY_BACK;
                slide_left = 0;
            }

            ev.value = 1;
            in_touch = 0;
            reset_gestures();
    } else if (ev.type == 3 && ev.code == 53) {
//    } else if(ev.type == EV_ABS && ev.code == ABS_MT_POSITION_X) {  //debugging
        old_x = touch_x;
        touch_x = ev.value;
        if (old_x != 0)
            diff_x += touch_x - old_x;

	if (touch_y < (gr_fb_height() - gr_get_height(surface))) {
            if (diff_x > (gr_fb_width() / 4)) {
#ifdef SK8S_DEBUG_UI
		printf("Gesture forward generated\n");
#endif
                slide_right = 1;
                //ev.code = KEY_ENTER;
                //ev.type = EV_KEY;
                reset_gestures();
    } else if(diff_x < ((gr_fb_width() / 4) * -1)) {
#ifdef SK8S_DEBUG_UI
		printf("Gesture back generated\n");
#endif
                slide_left = 1;
                //ev.code = KEY_BACK;
                //ev.type = EV_KEY;
                reset_gestures();
            }
        } else {
            input_buttons();
            //reset_gestures();
        }
//    } else if(ev.type == EV_ABS && ev.code == ABS_MT_POSITION_Y) {
    } else if (ev.type == 3 && ev.code == 54) {
        old_y = touch_y;
        touch_y = ev.value;
        if (old_y != 0)
            diff_y += touch_y - old_y;

    if (touch_y < (gr_fb_height() - gr_get_height(surface))) {
            if (diff_y > 25) {
#ifdef SK8S_DEBUG_UI
                printf("Gesture Down generated\n");
#endif
                ev.code = KEY_DOWN;
                ev.type = EV_KEY;
                reset_gestures();
	} else if (diff_y < -25) {
#ifdef SK8S_DEBUG_UI
                printf("Gesture Up generated\n");
#endif
                ev.code = KEY_UP;
                ev.type = EV_KEY;
                reset_gestures();
            }
        } else {
            input_buttons();
            //reset_gestures();
        }
    }
//END GWEEDO TOUCH CODE
