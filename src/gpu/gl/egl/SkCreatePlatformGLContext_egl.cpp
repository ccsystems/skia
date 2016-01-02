
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"

#include <GLES2/gl2.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

namespace {

// TODO: Share this class with ANGLE if/when it gets support for EGL_KHR_fence_sync.
class SkEGLFenceSync : public SkGpuFenceSync {
public:
    static SkEGLFenceSync* CreateIfSupported(EGLDisplay);

    SkPlatformGpuFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool flushAndWaitFence(SkPlatformGpuFence fence) const override;
    void deleteFence(SkPlatformGpuFence fence) const override;

private:
    SkEGLFenceSync(EGLDisplay display) : fDisplay(display) {}

    EGLDisplay                    fDisplay;

    typedef SkGpuFenceSync INHERITED;
};

class EGLGLContext : public SkGLContext  {
public:
    EGLGLContext(GrGLStandard forcedGpuAPI);
    ~EGLGLContext() override;

    GrEGLImage texture2DToEGLImage(GrGLuint texID) const override;
    void destroyEGLImage(GrEGLImage) const override;
    GrGLuint eglImageToExternalTexture(GrEGLImage) const override;
    SkGLContext* createNew() const override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    EGLContext fContext;
    EGLDisplay fDisplay;
    EGLSurface fSurface;
};

EGLGLContext::EGLGLContext(GrGLStandard forcedGpuAPI)
    : fContext(EGL_NO_CONTEXT)
    , fDisplay(EGL_NO_DISPLAY)
    , fSurface(EGL_NO_SURFACE) {
    static const EGLint kEGLContextAttribsForOpenGL[] = {
        EGL_NONE
    };

    static const EGLint kEGLContextAttribsForOpenGLES[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    static const struct {
        const EGLint* fContextAttribs;
        EGLenum fAPI;
        EGLint  fRenderableTypeBit;
        GrGLStandard fStandard;
    } kAPIs[] = {
        {   // OpenGL
            kEGLContextAttribsForOpenGL,
            EGL_OPENGL_API,
            EGL_OPENGL_BIT,
            kGL_GrGLStandard
        },
        {   // OpenGL ES. This seems to work for both ES2 and 3 (when available).
            kEGLContextAttribsForOpenGLES,
            EGL_OPENGL_ES_API,
            EGL_OPENGL_ES2_BIT,
            kGLES_GrGLStandard
        },
    };

    size_t apiLimit = SK_ARRAY_COUNT(kAPIs);
    size_t api = 0;
    if (forcedGpuAPI == kGL_GrGLStandard) {
        apiLimit = 1;
    } else if (forcedGpuAPI == kGLES_GrGLStandard) {
        api = 1;
    }
    SkASSERT(forcedGpuAPI == kNone_GrGLStandard || kAPIs[api].fStandard == forcedGpuAPI);

    SkAutoTUnref<const GrGLInterface> gl;

    for (; nullptr == gl.get() && api < apiLimit; ++api) {
        fDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        EGLint majorVersion;
        EGLint minorVersion;
        eglInitialize(fDisplay, &majorVersion, &minorVersion);

#if 0
        SkDebugf("VENDOR: %s\n", eglQueryString(fDisplay, EGL_VENDOR));
        SkDebugf("APIS: %s\n", eglQueryString(fDisplay, EGL_CLIENT_APIS));
        SkDebugf("VERSION: %s\n", eglQueryString(fDisplay, EGL_VERSION));
        SkDebugf("EXTENSIONS %s\n", eglQueryString(fDisplay, EGL_EXTENSIONS));
#endif

        if (!eglBindAPI(kAPIs[api].fAPI)) {
            continue;
        }

        EGLint numConfigs = 0;
        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, kAPIs[api].fRenderableTypeBit,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };

        EGLConfig surfaceConfig;
        if (!eglChooseConfig(fDisplay, configAttribs, &surfaceConfig, 1, &numConfigs)) {
            SkDebugf("eglChooseConfig failed. EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        if (0 == numConfigs) {
            SkDebugf("No suitable EGL config found.\n");
            continue;
        }

        fContext = eglCreateContext(fDisplay, surfaceConfig, nullptr, kAPIs[api].fContextAttribs);
        if (EGL_NO_CONTEXT == fContext) {
            SkDebugf("eglCreateContext failed.  EGL Error: 0x%08x\n", eglGetError());
            continue;
        }

        static const EGLint kSurfaceAttribs[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE
        };

        fSurface = eglCreatePbufferSurface(fDisplay, surfaceConfig, kSurfaceAttribs);
        if (EGL_NO_SURFACE == fSurface) {
            SkDebugf("eglCreatePbufferSurface failed. EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
            SkDebugf("eglMakeCurrent failed.  EGL Error: 0x%08x\n", eglGetError());
            this->destroyGLContext();
            continue;
        }

        gl.reset(GrGLCreateNativeInterface());
        if (nullptr == gl.get()) {
            SkDebugf("Failed to create gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        if (!gl->validate()) {
            SkDebugf("Failed to validate gl interface.\n");
            this->destroyGLContext();
            continue;
        }

        this->init(gl.detach(), SkEGLFenceSync::CreateIfSupported(fDisplay));
        break;
    }
}

EGLGLContext::~EGLGLContext() {
    this->teardown();
    this->destroyGLContext();
}

void EGLGLContext::destroyGLContext() {
    if (fDisplay) {
        eglMakeCurrent(fDisplay, 0, 0, 0);

        if (fContext) {
            eglDestroyContext(fDisplay, fContext);
            fContext = EGL_NO_CONTEXT;
        }

        if (fSurface) {
            eglDestroySurface(fDisplay, fSurface);
            fSurface = EGL_NO_SURFACE;
        }

        //TODO should we close the display?
        fDisplay = EGL_NO_DISPLAY;
    }
}

GrEGLImage EGLGLContext::texture2DToEGLImage(GrGLuint texID) const {
    if (!this->gl()->hasExtension("EGL_KHR_gl_texture_2D_image")) {
        return GR_EGL_NO_IMAGE;
    }
    GrEGLImage img;
    GrEGLint attribs[] = { GR_EGL_GL_TEXTURE_LEVEL, 0, GR_EGL_NONE };
    GrEGLClientBuffer clientBuffer = reinterpret_cast<GrEGLClientBuffer>(texID);
    GR_GL_CALL_RET(this->gl(), img,
                   EGLCreateImage(fDisplay, fContext, GR_EGL_GL_TEXTURE_2D, clientBuffer, attribs));
    return img;
}

void EGLGLContext::destroyEGLImage(GrEGLImage image) const {
    GR_GL_CALL(this->gl(), EGLDestroyImage(fDisplay, image));
}

GrGLuint EGLGLContext::eglImageToExternalTexture(GrEGLImage image) const {
    GrGLClearErr(this->gl());
    if (!this->gl()->hasExtension("GL_OES_EGL_image_external")) {
        return 0;
    }
    GrGLEGLImageTargetTexture2DProc glEGLImageTargetTexture2D = 
            (GrGLEGLImageTargetTexture2DProc) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2D) {
        return 0;
    }
    GrGLuint texID;
    glGenTextures(1, &texID);
    if (!texID) {
        return 0;
    }
    glBindTexture(GR_GL_TEXTURE_EXTERNAL, texID);
    if (glGetError() != GR_GL_NO_ERROR) {
        glDeleteTextures(1, &texID);
        return 0;
    }
    glEGLImageTargetTexture2D(GR_GL_TEXTURE_EXTERNAL, image);
    if (glGetError() != GR_GL_NO_ERROR) {
        glDeleteTextures(1, &texID);
        return 0;
    }
    return texID;
}

SkGLContext* EGLGLContext::createNew() const {
    SkGLContext* ctx = SkCreatePlatformGLContext(this->gl()->fStandard);
    if (ctx) {
        ctx->makeCurrent();
    }
    return ctx;
}

void EGLGLContext::onPlatformMakeCurrent() const {
    if (!eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Could not set the context.\n");
    }
}

void EGLGLContext::onPlatformSwapBuffers() const {
    if (!eglSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Could not complete eglSwapBuffers.\n");
    }
}

GrGLFuncPtr EGLGLContext::onPlatformGetProcAddress(const char* procName) const {
#ifdef SK_BUILD_FOR_RPI
    if(strcmp(procName, "glGetString") == 0) return (GrGLFuncPtr)glGetString;
    if(strcmp(procName, "glGetIntegerv") == 0) return (GrGLFuncPtr)glGetIntegerv;
    if(strcmp(procName, "glActiveTexture") == 0) return (GrGLFuncPtr)glActiveTexture;
    if(strcmp(procName, "glAttachShader") == 0) return (GrGLFuncPtr)glAttachShader;
    if(strcmp(procName, "glBindAttribLocation") == 0) return (GrGLFuncPtr)glBindAttribLocation;
    if(strcmp(procName, "glBindBuffer") == 0) return (GrGLFuncPtr)glBindBuffer;
    if(strcmp(procName, "glBindTexture") == 0) return (GrGLFuncPtr)glBindTexture;
    if(strcmp(procName, "glBlendColor") == 0) return (GrGLFuncPtr)glBlendColor;
    if(strcmp(procName, "glBlendEquation") == 0) return (GrGLFuncPtr)glBlendEquation;
    if(strcmp(procName, "glBlendFunc") == 0) return (GrGLFuncPtr)glBlendFunc;
    if(strcmp(procName, "glBufferData") == 0) return (GrGLFuncPtr)glBufferData;
    if(strcmp(procName, "glBufferSubData") == 0) return (GrGLFuncPtr)glBufferSubData;
    if(strcmp(procName, "glClear") == 0) return (GrGLFuncPtr)glClear;
    if(strcmp(procName, "glClearColor") == 0) return (GrGLFuncPtr)glClearColor;
    if(strcmp(procName, "glClearStencil") == 0) return (GrGLFuncPtr)glClearStencil;
    if(strcmp(procName, "glColorMask") == 0) return (GrGLFuncPtr)glColorMask;
    if(strcmp(procName, "glCompileShader") == 0) return (GrGLFuncPtr)glCompileShader;
    if(strcmp(procName, "glCompressedTexImage2D") == 0) return (GrGLFuncPtr)glCompressedTexImage2D;
    if(strcmp(procName, "glCompressedTexSubImage2D") == 0) return (GrGLFuncPtr)glCompressedTexSubImage2D;
    if(strcmp(procName, "glCopyTexSubImage2D") == 0) return (GrGLFuncPtr)glCopyTexSubImage2D;
    if(strcmp(procName, "glCreateProgram") == 0) return (GrGLFuncPtr)glCreateProgram;
    if(strcmp(procName, "glCreateShader") == 0) return (GrGLFuncPtr)glCreateShader;
    if(strcmp(procName, "glBindFramebuffer") == 0) return (GrGLFuncPtr)glBindFramebuffer;
    if(strcmp(procName, "glBindRenderbuffer") == 0) return (GrGLFuncPtr)glBindRenderbuffer;
    if(strcmp(procName, "glCheckFramebufferStatus") == 0) return (GrGLFuncPtr)glCheckFramebufferStatus;
    if(strcmp(procName, "glDeleteFramebuffers") == 0) return (GrGLFuncPtr)glDeleteFramebuffers;
    if(strcmp(procName, "glDeleteRenderbuffers") == 0) return (GrGLFuncPtr)glDeleteRenderbuffers;
    if(strcmp(procName, "glFramebufferRenderbuffer") == 0) return (GrGLFuncPtr)glFramebufferRenderbuffer;
    if(strcmp(procName, "glFramebufferTexture2D") == 0) return (GrGLFuncPtr)glFramebufferTexture2D;
    if(strcmp(procName, "glGenFramebuffers") == 0) return (GrGLFuncPtr)glGenFramebuffers;
    if(strcmp(procName, "glGenRenderbuffers") == 0) return (GrGLFuncPtr)glGenRenderbuffers;
    if(strcmp(procName, "glGetFramebufferAttachmentParameteriv") == 0) return (GrGLFuncPtr)glGetFramebufferAttachmentParameteriv;
    if(strcmp(procName, "glGetRenderbufferParameteriv") == 0) return (GrGLFuncPtr)glGetRenderbufferParameteriv;
    if(strcmp(procName, "glGetUniformLocation") == 0) return (GrGLFuncPtr)glGetUniformLocation;
    if(strcmp(procName, "glIsTexture") == 0) return (GrGLFuncPtr)glIsTexture;
    if(strcmp(procName, "glLineWidth") == 0) return (GrGLFuncPtr)glLineWidth;
    if(strcmp(procName, "glLinkProgram") == 0) return (GrGLFuncPtr)glLinkProgram;
    if(strcmp(procName, "glPixelStorei") == 0) return (GrGLFuncPtr)glPixelStorei;
    if(strcmp(procName, "glReadPixels") == 0) return (GrGLFuncPtr)glReadPixels;
    if(strcmp(procName, "glRenderbufferStorage") == 0) return (GrGLFuncPtr)glRenderbufferStorage;
    if(strcmp(procName, "glScissor") == 0) return (GrGLFuncPtr)glScissor;
    if(strcmp(procName, "glShaderSource") == 0) return (GrGLFuncPtr)glShaderSource;
    if(strcmp(procName, "glStencilFunc") == 0) return (GrGLFuncPtr)glStencilFunc;
    if(strcmp(procName, "glStencilFuncSeparate") == 0) return (GrGLFuncPtr)glStencilFuncSeparate;
    if(strcmp(procName, "glStencilMask") == 0) return (GrGLFuncPtr)glStencilMask;
    if(strcmp(procName, "glStencilMaskSeparate") == 0) return (GrGLFuncPtr)glStencilMaskSeparate;
    if(strcmp(procName, "glStencilOp") == 0) return (GrGLFuncPtr)glStencilOp;
    if(strcmp(procName, "glStencilOpSeparate") == 0) return (GrGLFuncPtr)glStencilOpSeparate;
    if(strcmp(procName, "glTexImage2D") == 0) return (GrGLFuncPtr)glTexImage2D;
    if(strcmp(procName, "glTexParameteri") == 0) return (GrGLFuncPtr)glTexParameteri;
#endif
    return eglGetProcAddress(procName);
}

static bool supports_egl_extension(EGLDisplay display, const char* extension) {
    size_t extensionLength = strlen(extension);
    const char* extensionsStr = eglQueryString(display, EGL_EXTENSIONS);
    while (const char* match = strstr(extensionsStr, extension)) {
        // Ensure the string we found is its own extension, not a substring of a larger extension
        // (e.g. GL_ARB_occlusion_query / GL_ARB_occlusion_query2).
        if ((match == extensionsStr || match[-1] == ' ') &&
            (match[extensionLength] == ' ' || match[extensionLength] == '\0')) {
            return true;
        }
        extensionsStr = match + extensionLength;
    }
    return false;
}

SkEGLFenceSync* SkEGLFenceSync::CreateIfSupported(EGLDisplay display) {
    if (!display || !supports_egl_extension(display, "EGL_KHR_fence_sync")) {
        return nullptr;
    }
    return new SkEGLFenceSync(display);
}

SkPlatformGpuFence SkEGLFenceSync::insertFence() const {
    return eglCreateSyncKHR(fDisplay, EGL_SYNC_FENCE_KHR, nullptr);
}

bool SkEGLFenceSync::flushAndWaitFence(SkPlatformGpuFence platformFence) const {
    EGLSyncKHR eglsync = static_cast<EGLSyncKHR>(platformFence);
    return EGL_CONDITION_SATISFIED_KHR == eglClientWaitSyncKHR(fDisplay,
                                                               eglsync,
                                                               EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                                               EGL_FOREVER_KHR);
}

void SkEGLFenceSync::deleteFence(SkPlatformGpuFence platformFence) const {
    EGLSyncKHR eglsync = static_cast<EGLSyncKHR>(platformFence);
    eglDestroySyncKHR(fDisplay, eglsync);
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    EGLGLContext* ctx = new EGLGLContext(forcedGpuAPI);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

