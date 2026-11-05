#include "app/app.h"

#include <stdbool.h>

#include <SDL/SDL.h>

#include "mui/input.h"
#include "mui/log.h"
#include "mui/gfx.h"
#include "mui/screen.h"
#include "ui/screen_counter.h"
#include "ui/screen_picker.h"
#include "ui/screen_splash.h"

/* Screens run in order: splash -> colour picker -> life counter. */
typedef enum { ST_SPLASH, ST_PICKER, ST_COUNTER } State;

typedef struct {
    Gfx gfx;
    State state;
    SplashScreen splash;
    PickerScreen picker;
    CounterScreen counter;
    bool running, dirty;
} App;

/* Moves on to the screen after the current one. Returns false on failure. */
static bool advance(App *app)
{
    switch (app->state) {
    case ST_SPLASH:
        if (!screen_picker_init(&app->picker, &app->gfx))
            return false;
        app->state = ST_PICKER;
        break;
    case ST_PICKER:
        if (!screen_counter_init(&app->counter, &app->gfx, app->picker.sel))
            return false;
        screen_picker_free(&app->picker);
        app->state = ST_COUNTER;
        break;
    case ST_COUNTER:
        break;
    }
    LOG_INFO("state %d", app->state);
    app->dirty = true;
    return true;
}

static void apply(App *app, ScreenResult r)
{
    if (r == SCREEN_EXIT)
        app->running = false;
    else if (r == SCREEN_DONE && !advance(app))
        app->running = false;
}

static void handle_button(App *app, Button btn, Uint32 now)
{
    ScreenResult r = SCREEN_OK;
    switch (app->state) {
    case ST_SPLASH:  r = screen_splash_handle(&app->splash, btn); break;
    case ST_PICKER:  r = screen_picker_handle(&app->picker, btn, now); break;
    case ST_COUNTER: r = screen_counter_handle(&app->counter, btn, now); break;
    }
    app->dirty = true;
    apply(app, r);
}

static void tick(App *app, Uint32 now)
{
    switch (app->state) {
    case ST_SPLASH: apply(app, screen_splash_tick(&app->splash, now)); break;
    case ST_PICKER: apply(app, screen_picker_tick(&app->picker, now)); break;
    case ST_COUNTER:
        if (screen_counter_tick(&app->counter, now))
            app->dirty = true;
        break;
    }
}

static void render(App *app, Uint32 now)
{
    switch (app->state) {
    case ST_SPLASH:  screen_splash_render(&app->splash, &app->gfx); break;
    case ST_PICKER:  screen_picker_render(&app->picker, &app->gfx); break;
    case ST_COUNTER: screen_counter_render(&app->counter, &app->gfx, now); break;
    }
}

int app_run(void)
{
    static App app; /* zeroed; big-ish, keep it off the stack */
    if (!gfx_init(&app.gfx)) {
        gfx_quit(&app.gfx);
        return 1;
    }

    app.state = ST_SPLASH;
    screen_splash_init(&app.splash);
    app.running = app.dirty = true;

    while (app.running) {
        SDL_Event ev;
        while (app.running && SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                app.running = false;
            } else if (ev.type == SDL_KEYDOWN) {
                Button btn = input_button_from_key(ev.key.keysym.sym);
                LOG_DEBUG("button %s", input_button_name(btn));
                handle_button(&app, btn, SDL_GetTicks());
            }
        }
        if (app.running)
            tick(&app, SDL_GetTicks());

        if (app.dirty && app.running) {
            render(&app, SDL_GetTicks());
            app.dirty = false;
        }
        SDL_Delay(16);
    }

    LOG_INFO("exiting");
    screen_picker_free(&app.picker);
    screen_counter_free(&app.counter);
    gfx_quit(&app.gfx);
    return 0;
}
