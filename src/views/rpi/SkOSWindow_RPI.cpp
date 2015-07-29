
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <SDL2/SDL.h>

#include "SkWindow.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkWindow.h"

static Uint32 USER_EVENT;
static SkKey find_skkey(SDL_Keycode src);

static void post_event(int code) {
    SDL_Event evt;
    evt.type = USER_EVENT;
    evt.user.code = code;
    evt.user.data1 = NULL;
    evt.user.data2 = NULL;
    SDL_PushEvent(&evt);
}

SkOSWindow::SkOSWindow(void*)
{
    USER_EVENT = SDL_RegisterEvents(1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window = SDL_CreateWindow("SKIA Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        0, 0, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if(!window)
    {
        SkDebugf("SDL could not create window: %s\n", SDL_GetError());
        return;
    }

    glcontext = SDL_GL_CreateContext(window);
    if(!glcontext)
    {
        SkDebugf("SDL could not create GL context: %s\n", SDL_GetError());
        return;
    }
}

SkOSWindow::~SkOSWindow() {
    if(window)
        SDL_DestroyWindow(window);
}

void SkOSWindow::initWindow(int requestedMSAASampleCount, AttachmentInfo* info) {
    int width, height;

    SDL_GL_SetSwapInterval(1);

    SDL_GetWindowSize(window, &width, &height);
    SkDebugf("Found window size %d x %d\n", width, height);

    this->resize(width, height);

    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    SDL_GL_SwapWindow(window);
}

void SkOSWindow::loop() {
    SDL_Event ev;

    while (SDL_WaitEvent(&ev)) {
        if (ev.type == SDL_QUIT)
            break;

        processEvent(ev);
    }
}

void SkOSWindow::processEvent(SDL_Event ev)
{
     if (ev.type == USER_EVENT) {
        if (ev.user.code == 0) {
            if (SkEvent::ProcessEvent()) {
                post_event(0);
            }
        } else if(ev.user.code == 1) {
            this->update(NULL);
        }
    }

    switch (ev.type) {
        case SDL_MOUSEMOTION:
            if (ev.motion.state == SDL_PRESSED) {
                this->handleClick(ev.motion.x, ev.motion.y,
                                   SkView::Click::kMoved_State, NULL, 0);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            this->handleClick(ev.button.x, ev.button.y,
                               ev.button.state == SDL_PRESSED ?
                               SkView::Click::kDown_State :
                               SkView::Click::kUp_State, NULL, 0);
            break;
        case SDL_KEYDOWN: {
            SkKey sk = find_skkey(ev.key.keysym.sym);
            if (kNONE_SkKey != sk) {
                if (ev.key.state == SDL_PRESSED) {
                    this->handleKey(sk);
                } else {
                    this->handleKeyUp(sk);
                }
            }
            break;
        }
    }
}

bool SkOSWindow::attach(SkBackEndTypes, int msaaSampleCount, AttachmentInfo* info)
{
    this->initWindow(msaaSampleCount, info);
    return true;
}

void SkOSWindow::detach() {
}

void SkOSWindow::present() {
    SDL_GL_SwapWindow(window);
}

void SkOSWindow::onHandleInval(const SkIRect& r) {
    post_event(1);
}

///////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue() {
    post_event(0);
}

static Uint32 timer_callback(Uint32 interval, void *param) {
    SkEvent::ServiceQueueTimer();
    return 0;
}

void SkEvent::SignalQueueTimer(SkMSec delay)
{
    if (delay) {
        SDL_AddTimer(delay, timer_callback, NULL);
    }
}

static SkKey find_skkey(SDL_Keycode src) {
    // this array must match the enum order in SkKey.h
    static const SDL_Keycode gKeys[] = {
        SDLK_UNKNOWN,
        SDLK_UNKNOWN,   // left softkey
        SDLK_UNKNOWN,   // right softkey
        SDLK_UNKNOWN,   // home
        SDLK_UNKNOWN,   // back
        SDLK_UNKNOWN,   // send
        SDLK_UNKNOWN,   // end
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
        SDLK_ASTERISK,
        SDLK_HASH,
        SDLK_UP,
        SDLK_DOWN,
        SDLK_LEFT,
        SDLK_RIGHT,
        SDLK_RETURN,    // OK
        SDLK_UNKNOWN,   // volume up
        SDLK_UNKNOWN,   // volume down
        SDLK_UNKNOWN,   // power
        SDLK_UNKNOWN,   // camera
    };

    const SDL_Keycode* array = gKeys;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gKeys); i++) {
        if (array[i] == src) {
            return static_cast<SkKey>(i);
        }
    }
    return kNONE_SkKey;
}
