
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <dlfcn.h>

static void *libgl;

static GrGLFuncPtr linux_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);

	if(libgl == NULL)
		libgl = dlopen("libGL.so", RTLD_LAZY);

	return (GrGLFuncPtr) dlsym(libgl, name);
}

const GrGLInterface* GrGLCreateNativeInterface() {
    return GrGLAssembleInterface(NULL, linux_get_gl_proc);
}
