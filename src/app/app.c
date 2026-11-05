#include "app/app.h"

#include <stdbool.h>

#include <SDL/SDL.h>

#include "mui/input.h"
#include "mui/log.h"
#include "mui/gfx.h"
#include "mui/screen.h"
#include "ui/screen_commander.h"
#include "ui/screen_counter.h"
#include "ui/screen_picker.h"
#include "ui/screen_splash.h"

/* Screens run in order:
 *   2 player:  splash (mode menu) -> colour picker -> life counter
 *   4 player:  splash -> commander
 * SELECT on either game screen goes back to the splash. */
typedef enum { ST_SPLASH, ST_PICKER, ST_COUNTER, ST_COMMANDER } State;

typedef struct {
    Gfx gfx;
    State state;
    SplashScreen splash;
    PickerScreen picker;
    CounterScreen counter;
    CommanderScreen commander;
    bool running, dirty;
} App;

/* Back to the mode menu, with the last mode still highlighted. */
static void to_splash(App *app)
{
    GameModeId mode = app->splash.mode;
    screen_splash_init(&app->splash, &app->gfx);
    app->splash.mode = mode;
    app->state = ST_SPLASH;
}

/* Moves on to the screen after the current one. Returns false on failure. */
static bool advance(App *app)
{
    const GameMode *mode = &GAME_MODES[app->splash.mode];
    switch (app->state) {
    case ST_SPLASH:
        if (mode->players == CMD_PLAYERS) {
            if (!screen_commander_init(&app->commander, &app->gfx, mode->start_life))
                return false;
            app->state = ST_COMMANDER;
        } else {
            if (!screen_picker_init(&app->picker, &app->gfx))
                return false;
            app->state = ST_PICKER;
        }
        screen_splash_free(&app->splash);
        break;
    case ST_PICKER:
        if (!screen_counter_init(&app->counter, &app->gfx, app->picker.sel,
                                 mode->start_life))
            return false;
        screen_picker_free(&app->picker);
        app->state = ST_COUNTER;
        break;
    case ST_COUNTER:
        screen_counter_free(&app->counter);
        to_splash(app);
        break;
    case ST_COMMANDER:
        screen_commander_free(&app->commander);
        to_splash(app);
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
    case ST_COMMANDER: r = screen_commander_handle(&app->commander, btn, now); break;
    }
    app->dirty = true;
    apply(app, r);
}

static void tick(App *app, Uint32 now)
{
    switch (app->state) {
    case ST_SPLASH: break; /* waits for a mode to be picked */
    case ST_PICKER: apply(app, screen_picker_tick(&app->picker, now)); break;
    case ST_COUNTER:
        if (screen_counter_tick(&app->counter, now))
            app->dirty = true;
        break;
    case ST_COMMANDER:
        if (screen_commander_tick(&app->commander, now))
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
    case ST_COMMANDER: screen_commander_render(&app->commander, &app->gfx, now); break;
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
    screen_splash_init(&app.splash, &app.gfx);
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
    screen_splash_free(&app.splash);
    screen_picker_free(&app.picker);
    screen_counter_free(&app.counter);
    screen_commander_free(&app.commander);
    gfx_quit(&app.gfx);
    return 0;
}
