#ifndef LC_UI_SCREEN_H
#define LC_UI_SCREEN_H

/* What a screen's input handler asks the app to do next. */
typedef enum {
    SCREEN_OK,   /* stay on this screen */
    SCREEN_DONE, /* this screen is finished, go to the next one */
    SCREEN_EXIT, /* quit the app */
} ScreenResult;

#endif
