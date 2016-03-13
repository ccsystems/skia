
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

static GrGLFuncPtr egl_get_gl_proc(void* ctx, const char name[]) {
#if defined(SK_BUILD_FOR_RPI) || defined(SK_BUILD_FOR_ODROID)
    if(strcmp(name, "glGetString") == 0) return (GrGLFuncPtr)glGetString;
    if(strcmp(name, "glGetIntegerv") == 0) return (GrGLFuncPtr)glGetIntegerv;
    if(strcmp(name, "glActiveTexture") == 0) return (GrGLFuncPtr)glActiveTexture;
    if(strcmp(name, "glAttachShader") == 0) return (GrGLFuncPtr)glAttachShader;
    if(strcmp(name, "glBindAttribLocation") == 0) return (GrGLFuncPtr)glBindAttribLocation;
    if(strcmp(name, "glBindBuffer") == 0) return (GrGLFuncPtr)glBindBuffer;
    if(strcmp(name, "glBindTexture") == 0) return (GrGLFuncPtr)glBindTexture;
    if(strcmp(name, "glBlendColor") == 0) return (GrGLFuncPtr)glBlendColor;
    if(strcmp(name, "glBlendEquation") == 0) return (GrGLFuncPtr)glBlendEquation;
    if(strcmp(name, "glBlendFunc") == 0) return (GrGLFuncPtr)glBlendFunc;
    if(strcmp(name, "glBufferData") == 0) return (GrGLFuncPtr)glBufferData;
    if(strcmp(name, "glBufferSubData") == 0) return (GrGLFuncPtr)glBufferSubData;
    if(strcmp(name, "glClear") == 0) return (GrGLFuncPtr)glClear;
    if(strcmp(name, "glClearColor") == 0) return (GrGLFuncPtr)glClearColor;
    if(strcmp(name, "glClearStencil") == 0) return (GrGLFuncPtr)glClearStencil;
    if(strcmp(name, "glColorMask") == 0) return (GrGLFuncPtr)glColorMask;
    if(strcmp(name, "glCompileShader") == 0) return (GrGLFuncPtr)glCompileShader;
    if(strcmp(name, "glCompressedTexImage2D") == 0) return (GrGLFuncPtr)glCompressedTexImage2D;
    if(strcmp(name, "glCompressedTexSubImage2D") == 0) return (GrGLFuncPtr)glCompressedTexSubImage2D;
    if(strcmp(name, "glCopyTexSubImage2D") == 0) return (GrGLFuncPtr)glCopyTexSubImage2D;
    if(strcmp(name, "glCreateProgram") == 0) return (GrGLFuncPtr)glCreateProgram;
    if(strcmp(name, "glCreateShader") == 0) return (GrGLFuncPtr)glCreateShader;
    if(strcmp(name, "glBindFramebuffer") == 0) return (GrGLFuncPtr)glBindFramebuffer;
    if(strcmp(name, "glBindRenderbuffer") == 0) return (GrGLFuncPtr)glBindRenderbuffer;
    if(strcmp(name, "glCheckFramebufferStatus") == 0) return (GrGLFuncPtr)glCheckFramebufferStatus;
    if(strcmp(name, "glDeleteFramebuffers") == 0) return (GrGLFuncPtr)glDeleteFramebuffers;
    if(strcmp(name, "glDeleteRenderbuffers") == 0) return (GrGLFuncPtr)glDeleteRenderbuffers;
    if(strcmp(name, "glFramebufferRenderbuffer") == 0) return (GrGLFuncPtr)glFramebufferRenderbuffer;
    if(strcmp(name, "glFramebufferTexture2D") == 0) return (GrGLFuncPtr)glFramebufferTexture2D;
    if(strcmp(name, "glGenFramebuffers") == 0) return (GrGLFuncPtr)glGenFramebuffers;
    if(strcmp(name, "glGenRenderbuffers") == 0) return (GrGLFuncPtr)glGenRenderbuffers;
    if(strcmp(name, "glGetFramebufferAttachmentParameteriv") == 0) return (GrGLFuncPtr)glGetFramebufferAttachmentParameteriv;
    if(strcmp(name, "glGetRenderbufferParameteriv") == 0) return (GrGLFuncPtr)glGetRenderbufferParameteriv;
    if(strcmp(name, "glGetUniformLocation") == 0) return (GrGLFuncPtr)glGetUniformLocation;
    if(strcmp(name, "glIsTexture") == 0) return (GrGLFuncPtr)glIsTexture;
    if(strcmp(name, "glLineWidth") == 0) return (GrGLFuncPtr)glLineWidth;
    if(strcmp(name, "glLinkProgram") == 0) return (GrGLFuncPtr)glLinkProgram;
    if(strcmp(name, "glPixelStorei") == 0) return (GrGLFuncPtr)glPixelStorei;
    if(strcmp(name, "glReadPixels") == 0) return (GrGLFuncPtr)glReadPixels;
    if(strcmp(name, "glRenderbufferStorage") == 0) return (GrGLFuncPtr)glRenderbufferStorage;
    if(strcmp(name, "glScissor") == 0) return (GrGLFuncPtr)glScissor;
    if(strcmp(name, "glShaderSource") == 0) return (GrGLFuncPtr)glShaderSource;
    if(strcmp(name, "glStencilFunc") == 0) return (GrGLFuncPtr)glStencilFunc;
    if(strcmp(name, "glStencilFuncSeparate") == 0) return (GrGLFuncPtr)glStencilFuncSeparate;
    if(strcmp(name, "glStencilMask") == 0) return (GrGLFuncPtr)glStencilMask;
    if(strcmp(name, "glStencilMaskSeparate") == 0) return (GrGLFuncPtr)glStencilMaskSeparate;
    if(strcmp(name, "glStencilOp") == 0) return (GrGLFuncPtr)glStencilOp;
    if(strcmp(name, "glStencilOpSeparate") == 0) return (GrGLFuncPtr)glStencilOpSeparate;
    if(strcmp(name, "glTexImage2D") == 0) return (GrGLFuncPtr)glTexImage2D;
    if(strcmp(name, "glTexParameteri") == 0) return (GrGLFuncPtr)glTexParameteri;
    if(strcmp(name, "glTexParameteriv") == 0) return (GrGLFuncPtr)glTexParameteriv;
    if(strcmp(name, "glTexSubImage2D") == 0) return (GrGLFuncPtr)glTexSubImage2D;
    if(strcmp(name, "glUniform1f") == 0) return (GrGLFuncPtr)glUniform1f;
    if(strcmp(name, "glUniform1fv") == 0) return (GrGLFuncPtr)glUniform1fv;
    if(strcmp(name, "glUniform1i") == 0) return (GrGLFuncPtr)glUniform1i;
    if(strcmp(name, "glUniform1iv") == 0) return (GrGLFuncPtr)glUniform1iv;
    if(strcmp(name, "glUniform2f") == 0) return (GrGLFuncPtr)glUniform2f;
    if(strcmp(name, "glUniform2f") == 0) return (GrGLFuncPtr)glUniform2f;
    if(strcmp(name, "glUniform2fv") == 0) return (GrGLFuncPtr)glUniform2fv;
    if(strcmp(name, "glUniform2fv") == 0) return (GrGLFuncPtr)glUniform2fv;
    if(strcmp(name, "glUniform2i") == 0) return (GrGLFuncPtr)glUniform2i;
    if(strcmp(name, "glUniform2i") == 0) return (GrGLFuncPtr)glUniform2i;
    if(strcmp(name, "glUniform2iv") == 0) return (GrGLFuncPtr)glUniform2iv;
    if(strcmp(name, "glUniform2iv") == 0) return (GrGLFuncPtr)glUniform2iv;
    if(strcmp(name, "glUniform3f") == 0) return (GrGLFuncPtr)glUniform3f;
    if(strcmp(name, "glUniform3f") == 0) return (GrGLFuncPtr)glUniform3f;
    if(strcmp(name, "glUniform3fv") == 0) return (GrGLFuncPtr)glUniform3fv;
    if(strcmp(name, "glUniform3fv") == 0) return (GrGLFuncPtr)glUniform3fv;
    if(strcmp(name, "glUniform3i") == 0) return (GrGLFuncPtr)glUniform3i;
    if(strcmp(name, "glUniform3i") == 0) return (GrGLFuncPtr)glUniform3i;
    if(strcmp(name, "glUniform3iv") == 0) return (GrGLFuncPtr)glUniform3iv;
    if(strcmp(name, "glUniform3iv") == 0) return (GrGLFuncPtr)glUniform3iv;
    if(strcmp(name, "glUniform4f") == 0) return (GrGLFuncPtr)glUniform4f;
    if(strcmp(name, "glUniform4f") == 0) return (GrGLFuncPtr)glUniform4f;
    if(strcmp(name, "glUniform4fv") == 0) return (GrGLFuncPtr)glUniform4fv;
    if(strcmp(name, "glUniform4fv") == 0) return (GrGLFuncPtr)glUniform4fv;
    if(strcmp(name, "glUniform4i") == 0) return (GrGLFuncPtr)glUniform4i;
    if(strcmp(name, "glUniform4i") == 0) return (GrGLFuncPtr)glUniform4i;
    if(strcmp(name, "glUniform4iv") == 0) return (GrGLFuncPtr)glUniform4iv;
    if(strcmp(name, "glUniform4iv") == 0) return (GrGLFuncPtr)glUniform4iv;
    if(strcmp(name, "glUniformMatrix2fv") == 0) return (GrGLFuncPtr)glUniformMatrix2fv;
    if(strcmp(name, "glUniformMatrix3fv") == 0) return (GrGLFuncPtr)glUniformMatrix3fv;
    if(strcmp(name, "glUniformMatrix4fv") == 0) return (GrGLFuncPtr)glUniformMatrix4fv;
    if(strcmp(name, "glUseProgram") == 0) return (GrGLFuncPtr)glUseProgram;
    if(strcmp(name, "glVertexAttrib1f") == 0) return (GrGLFuncPtr)glVertexAttrib1f;
    if(strcmp(name, "glVertexAttrib2fv") == 0) return (GrGLFuncPtr)glVertexAttrib2fv;
    if(strcmp(name, "glVertexAttrib3fv") == 0) return (GrGLFuncPtr)glVertexAttrib3fv;
    if(strcmp(name, "glVertexAttrib4fv") == 0) return (GrGLFuncPtr)glVertexAttrib4fv;
    if(strcmp(name, "glVertexAttribPointer") == 0) return (GrGLFuncPtr)glVertexAttribPointer;
    if(strcmp(name, "glViewport") == 0) return (GrGLFuncPtr)glViewport;
    if(strcmp(name, "glCullFace") == 0) return (GrGLFuncPtr)glCullFace;
    if(strcmp(name, "glDeleteBuffers") == 0) return (GrGLFuncPtr)glDeleteBuffers;
    if(strcmp(name, "glDeleteProgram") == 0) return (GrGLFuncPtr)glDeleteProgram;
    if(strcmp(name, "glDeleteShader") == 0) return (GrGLFuncPtr)glDeleteShader;
    if(strcmp(name, "glDeleteTextures") == 0) return (GrGLFuncPtr)glDeleteTextures;
    if(strcmp(name, "glDepthMask") == 0) return (GrGLFuncPtr)glDepthMask;
    if(strcmp(name, "glDisable") == 0) return (GrGLFuncPtr)glDisable;
    if(strcmp(name, "glDisableVertexAttribArray") == 0) return (GrGLFuncPtr)glDisableVertexAttribArray;
    if(strcmp(name, "glDrawArrays") == 0) return (GrGLFuncPtr)glDrawArrays;
    if(strcmp(name, "glDrawElements") == 0) return (GrGLFuncPtr)glDrawElements;
    if(strcmp(name, "glEnable") == 0) return (GrGLFuncPtr)glEnable;
    if(strcmp(name, "glEnableVertexAttribArray") == 0) return (GrGLFuncPtr)glEnableVertexAttribArray;
    if(strcmp(name, "glFinish") == 0) return (GrGLFuncPtr)glFinish;
    if(strcmp(name, "glFlush") == 0) return (GrGLFuncPtr)glFlush;
    if(strcmp(name, "glFrontFace") == 0) return (GrGLFuncPtr)glFrontFace;
    if(strcmp(name, "glGenBuffers") == 0) return (GrGLFuncPtr)glGenBuffers;
    if(strcmp(name, "glGenerateMipmap") == 0) return (GrGLFuncPtr)glGenerateMipmap;
    if(strcmp(name, "glGenTextures") == 0) return (GrGLFuncPtr)glGenTextures;
    if(strcmp(name, "glGetBufferParameteriv") == 0) return (GrGLFuncPtr)glGetBufferParameteriv;
    if(strcmp(name, "glGetError") == 0) return (GrGLFuncPtr)glGetError;
    if(strcmp(name, "glGetProgramInfoLog") == 0) return (GrGLFuncPtr)glGetProgramInfoLog;
    if(strcmp(name, "glGetProgramiv") == 0) return (GrGLFuncPtr)glGetProgramiv;
    if(strcmp(name, "glGetShaderInfoLog") == 0) return (GrGLFuncPtr)glGetShaderInfoLog;
    if(strcmp(name, "glGetShaderPrecisionFormat") == 0) return (GrGLFuncPtr)glGetShaderPrecisionFormat;
    if(strcmp(name, "glGetShaderiv") == 0) return (GrGLFuncPtr)glGetShaderiv;
#endif

    SkASSERT(nullptr == ctx);
    GrGLFuncPtr ptr = eglGetProcAddress(name);
    if (!ptr) {
        if (0 == strcmp("eglQueryString", name)) {
            return (GrGLFuncPtr)eglQueryString;
        } else if (0 == strcmp("eglGetCurrentDisplay", name)) {
            return (GrGLFuncPtr)eglGetCurrentDisplay;
        }
    }
    return ptr;
}

const GrGLInterface* GrGLCreateNativeInterface() {
    return GrGLAssembleInterface(nullptr, egl_get_gl_proc);
}
