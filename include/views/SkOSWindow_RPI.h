/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSWindow_RPI_DEFINED
#define SkOSWindow_RPI_DEFINED

#include "SkWindow.h"
#include <SDL2/SDL.h>

class SkEvent;

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*);
    ~SkOSWindow();

    void* getHWND() const { return (void*)NULL; }
    void* getDisplay() const { return (void*)NULL; }
    void loop();

    enum SkBackEndTypes {
        kNone_BackEndType,
        kNativeGL_BackEndType,
    };

	bool attach(SkBackEndTypes attachType, int msaaSampleCount, AttachmentInfo*);
	void detach();
	void present();

protected:
    virtual void onHandleInval(const SkIRect&);

private:
    void initWindow(int newMSAASampleCount, AttachmentInfo* info);
    void processEvent(SDL_Event ev);

    SDL_Window *window;
    SDL_GLContext glcontext;
    typedef SkWindow INHERITED;
};

#endif
