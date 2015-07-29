
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <bcm_host.h>
#include <interface/vmcs_host/vc_dispmanx.h>

#include "SkWindow.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkEvent.h"
#include "SkKey.h"
#include "SkWindow.h"

const int WIDTH = 500;
const int HEIGHT = 500;

static uint32_t screen_width, screen_height;
static EGL_DISPMANX_WINDOW_T nativewindow;
static DISPMANX_ELEMENT_HANDLE_T dispman_element;
static DISPMANX_DISPLAY_HANDLE_T dispman_display;
static DISPMANX_UPDATE_HANDLE_T dispman_update;
static VC_RECT_T dst_rect;
static VC_RECT_T src_rect;
static EGLContext fContext;
static EGLDisplay fDisplay;
static EGLSurface fSurface;

SkOSWindow::SkOSWindow(void*)
{
}

SkOSWindow::~SkOSWindow() {
}

void SkOSWindow::initWindow(int requestedMSAASampleCount, AttachmentInfo* info) {
   //already created the window?
    if(fSurface != 0)
        return;

    EGLint numConfigs = 0;
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    static const EGLint context_attributes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(fDisplay, NULL, NULL);

    EGLConfig surfaceConfig;
    if(!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
        SkDebugf("eglChooseConfig failed. EGL Error: 0x%08x\n", eglGetError());
        return;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    fContext = eglCreateContext(fDisplay, surfaceConfig, EGL_NO_CONTEXT, context_attributes);
    if(fContext == NULL)
        SkDebugf("bad contet\n");

    graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
    SkDebugf("Found screen size %d x %d\n", screen_width, screen_height);
    this->resize(screen_width, screen_height);

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = screen_width;
    dst_rect.height = screen_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = screen_width << 16;
    src_rect.height = screen_height << 16;

    dispman_display = vc_dispmanx_display_open(0 /* LCD */);
    dispman_update = vc_dispmanx_update_start(0);

    dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
        0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);

    nativewindow.element = dispman_element;
    nativewindow.width = screen_width;
    nativewindow.height = screen_height;
    vc_dispmanx_update_submit_sync(dispman_update);

    fSurface = eglCreateWindowSurface(fDisplay, surfaceConfig, &nativewindow, NULL);
    if(fSurface == NULL)
        SkDebugf("bad window\n");

    eglMakeCurrent(fDisplay, fSurface, fSurface, fContext);
    glViewport(0, 0, screen_width, screen_height);
    glClearColor(1, 0, 0, 1.0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    eglSwapBuffers(fDisplay, fSurface);
}

void SkOSWindow::loop() {
    for (;;) {
        SkEvent::ServiceQueueTimer();
        SkEvent::ProcessEvent();

        this->update(NULL);
        usleep(100);
    }
}

bool SkOSWindow::attach(SkBackEndTypes, int msaaSampleCount, AttachmentInfo* info)
{
    this->initWindow(msaaSampleCount, info);
    this->forceInvalAll();

    eglMakeCurrent(fDisplay, fSurface, fSurface, fContext);

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    eglSwapBuffers(fDisplay, fSurface);
    return true;
}

void SkOSWindow::detach() {
}

void SkOSWindow::present() {
    eglSwapBuffers(fDisplay, fSurface);
}

///////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue() {
    // nothing to do, since we spin on our event-queue, polling for XPending
}

void SkEvent::SignalQueueTimer(SkMSec delay) {
    // just need to record the delay time. We handle waking up for it in
    // MyXNextEventWithDelay()
    //gTimerDelay = delay;
}
