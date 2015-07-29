/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSWindow_RPI_DEFINED
#define SkOSWindow_RPI_DEFINED

#include "SkWindow.h"

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

private:
    void initWindow(int newMSAASampleCount, AttachmentInfo* info);

    typedef SkWindow INHERITED;
};

#endif
