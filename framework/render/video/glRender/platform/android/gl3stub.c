/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <EGL/egl.h>
#include "gl3stub.h"

GLboolean gl3stubInit()
{
#define FIND_PROC(s) s = (void*)eglGetProcAddress(#s)
    FIND_PROC(glReadBuffer);
    FIND_PROC(glDrawRangeElements);
    FIND_PROC(glTexImage3D);
    FIND_PROC(glTexSubImage3D);
    FIND_PROC(glCopyTexSubImage3D);
    FIND_PROC(glCompressedTexImage3D);
    FIND_PROC(glCompressedTexSubImage3D);
    FIND_PROC(glGenQueries);
    FIND_PROC(glDeleteQueries);
    FIND_PROC(glIsQuery);
    FIND_PROC(glBeginQuery);
    FIND_PROC(glEndQuery);
    FIND_PROC(glGetQueryiv);
    FIND_PROC(glGetQueryObjectuiv);
    FIND_PROC(glUnmapBuffer);
    FIND_PROC(glGetBufferPointerv);
    FIND_PROC(glDrawBuffers);
    FIND_PROC(glUniformMatrix2x3fv);
    FIND_PROC(glUniformMatrix3x2fv);
    FIND_PROC(glUniformMatrix2x4fv);
    FIND_PROC(glUniformMatrix4x2fv);
    FIND_PROC(glUniformMatrix3x4fv);
    FIND_PROC(glUniformMatrix4x3fv);
    FIND_PROC(glBlitFramebuffer);
    FIND_PROC(glRenderbufferStorageMultisample);
    FIND_PROC(glFramebufferTextureLayer);
    FIND_PROC(glMapBufferRange);
    FIND_PROC(glFlushMappedBufferRange);
    FIND_PROC(glBindVertexArray);
    FIND_PROC(glDeleteVertexArrays);
    FIND_PROC(glGenVertexArrays);
    FIND_PROC(glIsVertexArray);
    FIND_PROC(glGetIntegeri_v);
    FIND_PROC(glBeginTransformFeedback);
    FIND_PROC(glEndTransformFeedback);
    FIND_PROC(glBindBufferRange);
    FIND_PROC(glBindBufferBase);
    FIND_PROC(glTransformFeedbackVaryings);
    FIND_PROC(glGetTransformFeedbackVarying);
    FIND_PROC(glVertexAttribIPointer);
    FIND_PROC(glGetVertexAttribIiv);
    FIND_PROC(glGetVertexAttribIuiv);
    FIND_PROC(glVertexAttribI4i);
    FIND_PROC(glVertexAttribI4ui);
    FIND_PROC(glVertexAttribI4iv);
    FIND_PROC(glVertexAttribI4uiv);
    FIND_PROC(glGetUniformuiv);
    FIND_PROC(glGetFragDataLocation);
    FIND_PROC(glUniform1ui);
    FIND_PROC(glUniform2ui);
    FIND_PROC(glUniform3ui);
    FIND_PROC(glUniform4ui);
    FIND_PROC(glUniform1uiv);
    FIND_PROC(glUniform2uiv);
    FIND_PROC(glUniform3uiv);
    FIND_PROC(glUniform4uiv);
    FIND_PROC(glClearBufferiv);
    FIND_PROC(glClearBufferuiv);
    FIND_PROC(glClearBufferfv);
    FIND_PROC(glClearBufferfi);
    FIND_PROC(glGetStringi);
    FIND_PROC(glCopyBufferSubData);
    FIND_PROC(glGetUniformIndices);
    FIND_PROC(glGetActiveUniformsiv);
    FIND_PROC(glGetUniformBlockIndex);
    FIND_PROC(glGetActiveUniformBlockiv);
    FIND_PROC(glGetActiveUniformBlockName);
    FIND_PROC(glUniformBlockBinding);
    FIND_PROC(glDrawArraysInstanced);
    FIND_PROC(glDrawElementsInstanced);
    FIND_PROC(glFenceSync);
    FIND_PROC(glIsSync);
    FIND_PROC(glDeleteSync);
    FIND_PROC(glClientWaitSync);
    FIND_PROC(glWaitSync);
    FIND_PROC(glGetInteger64v);
    FIND_PROC(glGetSynciv);
    FIND_PROC(glGetInteger64i_v);
    FIND_PROC(glGetBufferParameteri64v);
    FIND_PROC(glGenSamplers);
    FIND_PROC(glDeleteSamplers);
    FIND_PROC(glIsSampler);
    FIND_PROC(glBindSampler);
    FIND_PROC(glSamplerParameteri);
    FIND_PROC(glSamplerParameteriv);
    FIND_PROC(glSamplerParameterf);
    FIND_PROC(glSamplerParameterfv);
    FIND_PROC(glGetSamplerParameteriv);
    FIND_PROC(glGetSamplerParameterfv);
    FIND_PROC(glVertexAttribDivisor);
    FIND_PROC(glBindTransformFeedback);
    FIND_PROC(glDeleteTransformFeedbacks);
    FIND_PROC(glGenTransformFeedbacks);
    FIND_PROC(glIsTransformFeedback);
    FIND_PROC(glPauseTransformFeedback);
    FIND_PROC(glResumeTransformFeedback);
    FIND_PROC(glGetProgramBinary);
    FIND_PROC(glProgramBinary);
    FIND_PROC(glProgramParameteri);
    FIND_PROC(glInvalidateFramebuffer);
    FIND_PROC(glInvalidateSubFramebuffer);
    FIND_PROC(glTexStorage2D);
    FIND_PROC(glTexStorage3D);
    FIND_PROC(glGetInternalformativ);
    FIND_PROC(glDispatchCompute);
    FIND_PROC(glDispatchComputeIndirect);
    FIND_PROC(glDrawArraysIndirect);
    FIND_PROC(glDrawElementsIndirect);
    FIND_PROC(glFramebufferParameteri);
    FIND_PROC(glGetFramebufferParameteriv);
    FIND_PROC(glGetProgramInterfaceiv);
    FIND_PROC(glGetProgramResourceIndex);
    FIND_PROC(glGetProgramResourceName);
    FIND_PROC(glGetProgramResourceiv);
    FIND_PROC(glGetProgramResourceLocation);
    FIND_PROC(glUseProgramStages);
    FIND_PROC(glActiveShaderProgram);
    FIND_PROC(glCreateShaderProgramv);
    FIND_PROC(glBindProgramPipeline);
    FIND_PROC(glDeleteProgramPipelines);
    FIND_PROC(glGenProgramPipelines);
    FIND_PROC(glIsProgramPipeline);
    FIND_PROC(glGetProgramPipelineiv);
    FIND_PROC(glProgramUniform1i);
    FIND_PROC(glProgramUniform2i);
    FIND_PROC(glProgramUniform3i);
    FIND_PROC(glProgramUniform4i);
    FIND_PROC(glProgramUniform1ui);
    FIND_PROC(glProgramUniform2ui);
    FIND_PROC(glProgramUniform3ui);
    FIND_PROC(glProgramUniform4ui);
    FIND_PROC(glProgramUniform1f);
    FIND_PROC(glProgramUniform2f);
    FIND_PROC(glProgramUniform3f);
    FIND_PROC(glProgramUniform4f);
    FIND_PROC(glProgramUniform1iv);
    FIND_PROC(glProgramUniform2iv);
    FIND_PROC(glProgramUniform3iv);
    FIND_PROC(glProgramUniform4iv);
    FIND_PROC(glProgramUniform1uiv);
    FIND_PROC(glProgramUniform2uiv);
    FIND_PROC(glProgramUniform3uiv);
    FIND_PROC(glProgramUniform4uiv);
    FIND_PROC(glProgramUniform1fv);
    FIND_PROC(glProgramUniform2fv);
    FIND_PROC(glProgramUniform3fv);
    FIND_PROC(glProgramUniform4fv);
    FIND_PROC(glProgramUniformMatrix2fv);
    FIND_PROC(glProgramUniformMatrix3fv);
    FIND_PROC(glProgramUniformMatrix4fv);
    FIND_PROC(glProgramUniformMatrix2x3fv);
    FIND_PROC(glProgramUniformMatrix3x2fv);
    FIND_PROC(glProgramUniformMatrix2x4fv);
    FIND_PROC(glProgramUniformMatrix4x2fv);
    FIND_PROC(glProgramUniformMatrix3x4fv);
    FIND_PROC(glProgramUniformMatrix4x3fv);
    FIND_PROC(glValidateProgramPipeline);
    FIND_PROC(glGetProgramPipelineInfoLog);
    FIND_PROC(glBindImageTexture);
    FIND_PROC(glGetBooleani_v);
    FIND_PROC(glMemoryBarrier);
    FIND_PROC(glMemoryBarrierByRegion);
    FIND_PROC(glTexStorage2DMultisample);
    FIND_PROC(glGetMultisamplefv);
    FIND_PROC(glSampleMaski);
    FIND_PROC(glGetTexLevelParameteriv);
    FIND_PROC(glGetTexLevelParameterfv);
    FIND_PROC(glBindVertexBuffer);
    FIND_PROC(glVertexAttribFormat);
    FIND_PROC(glVertexAttribIFormat);
    FIND_PROC(glVertexAttribBinding);
    FIND_PROC(glVertexBindingDivisor);
    FIND_PROC(glBlendBarrier);
    FIND_PROC(glCopyImageSubData);
    FIND_PROC(glDebugMessageControl);
    FIND_PROC(glDebugMessageInsert);
    FIND_PROC(glDebugMessageCallback);
    FIND_PROC(glGetDebugMessageLog);
    FIND_PROC(glPushDebugGroup);
    FIND_PROC(glPopDebugGroup);
    FIND_PROC(glObjectLabel);
    FIND_PROC(glGetObjectLabel);
    FIND_PROC(glObjectPtrLabel);
    FIND_PROC(glGetObjectPtrLabel);
    FIND_PROC(glGetPointerv);
    FIND_PROC(glEnablei);
    FIND_PROC(glDisablei);
    FIND_PROC(glBlendEquationi);
    FIND_PROC(glBlendEquationSeparatei);
    FIND_PROC(glBlendFunci);
    FIND_PROC(glBlendFuncSeparatei);
    FIND_PROC(glColorMaski);
    FIND_PROC(glIsEnabledi);
    FIND_PROC(glDrawElementsBaseVertex);
    FIND_PROC(glDrawRangeElementsBaseVertex);
    FIND_PROC(glDrawElementsInstancedBaseVertex);
    FIND_PROC(glFramebufferTexture);
    FIND_PROC(glPrimitiveBoundingBox);
    FIND_PROC(glGetGraphicsResetStatus);
    FIND_PROC(glReadnPixels);
    FIND_PROC(glGetnUniformfv);
    FIND_PROC(glGetnUniformiv);
    FIND_PROC(glGetnUniformuiv);
    FIND_PROC(glMinSampleShading);
    FIND_PROC(glPatchParameteri);
    FIND_PROC(glTexParameterIiv);
    FIND_PROC(glTexParameterIuiv);
    FIND_PROC(glGetTexParameterIiv);
    FIND_PROC(glGetTexParameterIuiv);
    FIND_PROC(glSamplerParameterIiv);
    FIND_PROC(glSamplerParameterIuiv);
    FIND_PROC(glGetSamplerParameterIiv);
    FIND_PROC(glGetSamplerParameterIuiv);
    FIND_PROC(glTexBuffer);
    FIND_PROC(glTexBufferRange);
    FIND_PROC(glTexStorage3DMultisample);
#undef FIND_PROC

    if (!glReadBuffer ||
            !glDrawRangeElements ||
            !glTexImage3D ||
            !glTexSubImage3D ||
            !glCopyTexSubImage3D ||
            !glCompressedTexImage3D ||
            !glCompressedTexSubImage3D ||
            !glGenQueries ||
            !glDeleteQueries ||
            !glIsQuery ||
            !glBeginQuery ||
            !glEndQuery ||
            !glGetQueryiv ||
            !glGetQueryObjectuiv ||
            !glUnmapBuffer ||
            !glGetBufferPointerv ||
            !glDrawBuffers ||
            !glUniformMatrix2x3fv ||
            !glUniformMatrix3x2fv ||
            !glUniformMatrix2x4fv ||
            !glUniformMatrix4x2fv ||
            !glUniformMatrix3x4fv ||
            !glUniformMatrix4x3fv ||
            !glBlitFramebuffer ||
            !glRenderbufferStorageMultisample ||
            !glFramebufferTextureLayer ||
            !glMapBufferRange ||
            !glFlushMappedBufferRange ||
            !glBindVertexArray ||
            !glDeleteVertexArrays ||
            !glGenVertexArrays ||
            !glIsVertexArray ||
            !glGetIntegeri_v ||
            !glBeginTransformFeedback ||
            !glEndTransformFeedback ||
            !glBindBufferRange ||
            !glBindBufferBase ||
            !glTransformFeedbackVaryings ||
            !glGetTransformFeedbackVarying ||
            !glVertexAttribIPointer ||
            !glGetVertexAttribIiv ||
            !glGetVertexAttribIuiv ||
            !glVertexAttribI4i ||
            !glVertexAttribI4ui ||
            !glVertexAttribI4iv ||
            !glVertexAttribI4uiv ||
            !glGetUniformuiv ||
            !glGetFragDataLocation ||
            !glUniform1ui ||
            !glUniform2ui ||
            !glUniform3ui ||
            !glUniform4ui ||
            !glUniform1uiv ||
            !glUniform2uiv ||
            !glUniform3uiv ||
            !glUniform4uiv ||
            !glClearBufferiv ||
            !glClearBufferuiv ||
            !glClearBufferfv ||
            !glClearBufferfi ||
            !glGetStringi ||
            !glCopyBufferSubData ||
            !glGetUniformIndices ||
            !glGetActiveUniformsiv ||
            !glGetUniformBlockIndex ||
            !glGetActiveUniformBlockiv ||
            !glGetActiveUniformBlockName ||
            !glUniformBlockBinding ||
            !glDrawArraysInstanced ||
            !glDrawElementsInstanced ||
            !glFenceSync ||
            !glIsSync ||
            !glDeleteSync ||
            !glClientWaitSync ||
            !glWaitSync ||
            !glGetInteger64v ||
            !glGetSynciv ||
            !glGetInteger64i_v ||
            !glGetBufferParameteri64v ||
            !glGenSamplers ||
            !glDeleteSamplers ||
            !glIsSampler ||
            !glBindSampler ||
            !glSamplerParameteri ||
            !glSamplerParameteriv ||
            !glSamplerParameterf ||
            !glSamplerParameterfv ||
            !glGetSamplerParameteriv ||
            !glGetSamplerParameterfv ||
            !glVertexAttribDivisor ||
            !glBindTransformFeedback ||
            !glDeleteTransformFeedbacks ||
            !glGenTransformFeedbacks ||
            !glIsTransformFeedback ||
            !glPauseTransformFeedback ||
            !glResumeTransformFeedback ||
            !glGetProgramBinary ||
            !glProgramBinary ||
            !glProgramParameteri ||
            !glInvalidateFramebuffer ||
            !glInvalidateSubFramebuffer ||
            !glTexStorage2D ||
            !glTexStorage3D ||
            !glGetInternalformativ) {
        return GL_ES_VERSION_2_0;
    }

    if (!glDispatchCompute ||
            !glDispatchComputeIndirect ||
            !glDrawArraysIndirect ||
            !glDrawElementsIndirect ||
            !glFramebufferParameteri ||
            !glGetFramebufferParameteriv ||
            !glGetProgramInterfaceiv ||
            !glGetProgramResourceIndex ||
            !glGetProgramResourceName ||
            !glGetProgramResourceiv ||
            !glGetProgramResourceLocation ||
            !glUseProgramStages ||
            !glActiveShaderProgram ||
            !glCreateShaderProgramv ||
            !glBindProgramPipeline ||
            !glDeleteProgramPipelines ||
            !glGenProgramPipelines ||
            !glIsProgramPipeline ||
            !glGetProgramPipelineiv ||
            !glProgramUniform1i ||
            !glProgramUniform2i ||
            !glProgramUniform3i ||
            !glProgramUniform4i ||
            !glProgramUniform1ui ||
            !glProgramUniform2ui ||
            !glProgramUniform3ui ||
            !glProgramUniform4ui ||
            !glProgramUniform1f ||
            !glProgramUniform2f ||
            !glProgramUniform3f ||
            !glProgramUniform4f ||
            !glProgramUniform1iv ||
            !glProgramUniform2iv ||
            !glProgramUniform3iv ||
            !glProgramUniform4iv ||
            !glProgramUniform1uiv ||
            !glProgramUniform2uiv ||
            !glProgramUniform3uiv ||
            !glProgramUniform4uiv ||
            !glProgramUniform1fv ||
            !glProgramUniform2fv ||
            !glProgramUniform3fv ||
            !glProgramUniform4fv ||
            !glProgramUniformMatrix2fv ||
            !glProgramUniformMatrix3fv ||
            !glProgramUniformMatrix4fv ||
            !glProgramUniformMatrix2x3fv ||
            !glProgramUniformMatrix3x2fv ||
            !glProgramUniformMatrix2x4fv ||
            !glProgramUniformMatrix4x2fv ||
            !glProgramUniformMatrix3x4fv ||
            !glProgramUniformMatrix4x3fv ||
            !glValidateProgramPipeline ||
            !glGetProgramPipelineInfoLog ||
            !glBindImageTexture ||
            !glGetBooleani_v ||
            !glMemoryBarrier ||
            !glMemoryBarrierByRegion ||
            !glTexStorage2DMultisample ||
            !glGetMultisamplefv ||
            !glSampleMaski ||
            !glGetTexLevelParameteriv ||
            !glGetTexLevelParameterfv ||
            !glBindVertexBuffer ||
            !glVertexAttribFormat ||
            !glVertexAttribIFormat ||
            !glVertexAttribBinding ||
            !glVertexBindingDivisor) {
        return GL_ES_VERSION_3_0;
    }

    if (!glBlendBarrier ||
            !glCopyImageSubData ||
            !glDebugMessageControl ||
            !glDebugMessageInsert ||
            !glDebugMessageCallback ||
            !glGetDebugMessageLog ||
            !glPushDebugGroup ||
            !glPopDebugGroup ||
            !glObjectLabel ||
            !glGetObjectLabel ||
            !glObjectPtrLabel ||
            !glGetObjectPtrLabel ||
            !glGetPointerv ||
            !glEnablei ||
            !glDisablei ||
            !glBlendEquationi ||
            !glBlendEquationSeparatei ||
            !glBlendFunci ||
            !glBlendFuncSeparatei ||
            !glColorMaski ||
            !glIsEnabledi ||
            !glDrawElementsBaseVertex ||
            !glDrawRangeElementsBaseVertex ||
            !glDrawElementsInstancedBaseVertex ||
            !glFramebufferTexture ||
            !glPrimitiveBoundingBox ||
            !glGetGraphicsResetStatus ||
            !glReadnPixels ||
            !glGetnUniformfv ||
            !glGetnUniformiv ||
            !glGetnUniformuiv ||
            !glMinSampleShading ||
            !glPatchParameteri ||
            !glTexParameterIiv ||
            !glTexParameterIuiv ||
            !glGetTexParameterIiv ||
            !glGetTexParameterIuiv ||
            !glSamplerParameterIiv ||
            !glSamplerParameterIuiv ||
            !glGetSamplerParameterIiv ||
            !glGetSamplerParameterIuiv ||
            !glTexBuffer ||
            !glTexBufferRange ||
            !glTexStorage3DMultisample) {
        return GL_ES_VERSION_3_1;
    }

    return GL_ES_VERSION_3_2;
}

/* Function pointer definitions */
/* GL ES 3.0 */
GL_APICALL void           (* GL_APIENTRY glReadBuffer) (GLenum mode);
GL_APICALL void           (* GL_APIENTRY glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
        const GLvoid *indices);
GL_APICALL void           (* GL_APIENTRY glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
        GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void           (* GL_APIENTRY glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
        GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void           (* GL_APIENTRY glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
        GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void           (* GL_APIENTRY glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width,
        GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
GL_APICALL void           (* GL_APIENTRY glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset,
        GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GL_APICALL void           (* GL_APIENTRY glGenQueries) (GLsizei n, GLuint *ids);
GL_APICALL void           (* GL_APIENTRY glDeleteQueries) (GLsizei n, const GLuint *ids);
GL_APICALL GLboolean      (* GL_APIENTRY glIsQuery) (GLuint id);
GL_APICALL void           (* GL_APIENTRY glBeginQuery) (GLenum target, GLuint id);
GL_APICALL void           (* GL_APIENTRY glEndQuery) (GLenum target);
GL_APICALL void           (* GL_APIENTRY glGetQueryiv) (GLenum target, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint *params);
GL_APICALL GLboolean      (* GL_APIENTRY glUnmapBuffer) (GLenum target);
GL_APICALL void           (* GL_APIENTRY glGetBufferPointerv) (GLenum target, GLenum pname, GLvoid **params);
GL_APICALL void           (* GL_APIENTRY glDrawBuffers) (GLsizei n, const GLenum *bufs);
GL_APICALL void           (* GL_APIENTRY glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
        GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GL_APICALL void           (* GL_APIENTRY glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat,
        GLsizei width, GLsizei height);
GL_APICALL void           (* GL_APIENTRY glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level,
        GLint layer);
GL_APICALL GLvoid        *(* GL_APIENTRY glMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GL_APICALL void           (* GL_APIENTRY glFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
GL_APICALL void           (* GL_APIENTRY glBindVertexArray) (GLuint array);
GL_APICALL void           (* GL_APIENTRY glDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
GL_APICALL void           (* GL_APIENTRY glGenVertexArrays) (GLsizei n, GLuint *arrays);
GL_APICALL GLboolean      (* GL_APIENTRY glIsVertexArray) (GLuint array);
GL_APICALL void           (* GL_APIENTRY glGetIntegeri_v) (GLenum target, GLuint index, GLint *data);
GL_APICALL void           (* GL_APIENTRY glBeginTransformFeedback) (GLenum primitiveMode);
GL_APICALL void           (* GL_APIENTRY glEndTransformFeedback) (void);
GL_APICALL void           (* GL_APIENTRY glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GL_APICALL void           (* GL_APIENTRY glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
GL_APICALL void           (* GL_APIENTRY glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar *const *varyings,
        GLenum bufferMode);
GL_APICALL void           (* GL_APIENTRY glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length,
        GLsizei *size, GLenum *type, GLchar *name);
GL_APICALL void           (* GL_APIENTRY glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride,
        const GLvoid *pointer);
GL_APICALL void           (* GL_APIENTRY glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint *params);
GL_APICALL void           (* GL_APIENTRY glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
GL_APICALL void           (* GL_APIENTRY glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GL_APICALL void           (* GL_APIENTRY glVertexAttribI4iv) (GLuint index, const GLint *v);
GL_APICALL void           (* GL_APIENTRY glVertexAttribI4uiv) (GLuint index, const GLuint *v);
GL_APICALL void           (* GL_APIENTRY glGetUniformuiv) (GLuint program, GLint location, GLuint *params);
GL_APICALL GLint          (* GL_APIENTRY glGetFragDataLocation) (GLuint program, const GLchar *name);
GL_APICALL void           (* GL_APIENTRY glUniform1ui) (GLint location, GLuint v0);
GL_APICALL void           (* GL_APIENTRY glUniform2ui) (GLint location, GLuint v0, GLuint v1);
GL_APICALL void           (* GL_APIENTRY glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void           (* GL_APIENTRY glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void           (* GL_APIENTRY glUniform1uiv) (GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glUniform2uiv) (GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glUniform3uiv) (GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glUniform4uiv) (GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint *value);
GL_APICALL void           (* GL_APIENTRY glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
GL_APICALL const GLubyte *(* GL_APIENTRY glGetStringi) (GLenum name, GLuint index);
GL_APICALL void           (* GL_APIENTRY glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset,
        GLintptr writeOffset, GLsizeiptr size);
GL_APICALL void           (* GL_APIENTRY glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames,
        GLuint *uniformIndices);
GL_APICALL void           (* GL_APIENTRY glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices,
        GLenum pname, GLint *params);
GL_APICALL GLuint         (* GL_APIENTRY glGetUniformBlockIndex) (GLuint program, const GLchar *uniformBlockName);
GL_APICALL void           (* GL_APIENTRY glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize,
        GLsizei *length, GLchar *uniformBlockName);
GL_APICALL void           (* GL_APIENTRY glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
GL_APICALL void           (* GL_APIENTRY glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
GL_APICALL void           (* GL_APIENTRY glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices,
        GLsizei instanceCount);
GL_APICALL GLsync         (* GL_APIENTRY glFenceSync) (GLenum condition, GLbitfield flags);
GL_APICALL GLboolean      (* GL_APIENTRY glIsSync) (GLsync sync);
GL_APICALL void           (* GL_APIENTRY glDeleteSync) (GLsync sync);
GL_APICALL GLenum         (* GL_APIENTRY glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
GL_APICALL void           (* GL_APIENTRY glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
GL_APICALL void           (* GL_APIENTRY glGetInteger64v) (GLenum pname, GLint64 *params);
GL_APICALL void           (* GL_APIENTRY glGetSynciv) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
GL_APICALL void           (* GL_APIENTRY glGetInteger64i_v) (GLenum target, GLuint index, GLint64 *data);
GL_APICALL void           (* GL_APIENTRY glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64 *params);
GL_APICALL void           (* GL_APIENTRY glGenSamplers) (GLsizei count, GLuint *samplers);
GL_APICALL void           (* GL_APIENTRY glDeleteSamplers) (GLsizei count, const GLuint *samplers);
GL_APICALL GLboolean      (* GL_APIENTRY glIsSampler) (GLuint sampler);
GL_APICALL void           (* GL_APIENTRY glBindSampler) (GLuint unit, GLuint sampler);
GL_APICALL void           (* GL_APIENTRY glSamplerParameteri) (GLuint sampler, GLenum pname, GLint param);
GL_APICALL void           (* GL_APIENTRY glSamplerParameteriv) (GLuint sampler, GLenum pname, const GLint *param);
GL_APICALL void           (* GL_APIENTRY glSamplerParameterf) (GLuint sampler, GLenum pname, GLfloat param);
GL_APICALL void           (* GL_APIENTRY glSamplerParameterfv) (GLuint sampler, GLenum pname, const GLfloat *param);
GL_APICALL void           (* GL_APIENTRY glGetSamplerParameteriv) (GLuint sampler, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetSamplerParameterfv) (GLuint sampler, GLenum pname, GLfloat *params);
GL_APICALL void           (* GL_APIENTRY glVertexAttribDivisor) (GLuint index, GLuint divisor);
GL_APICALL void           (* GL_APIENTRY glBindTransformFeedback) (GLenum target, GLuint id);
GL_APICALL void           (* GL_APIENTRY glDeleteTransformFeedbacks) (GLsizei n, const GLuint *ids);
GL_APICALL void           (* GL_APIENTRY glGenTransformFeedbacks) (GLsizei n, GLuint *ids);
GL_APICALL GLboolean      (* GL_APIENTRY glIsTransformFeedback) (GLuint id);
GL_APICALL void           (* GL_APIENTRY glPauseTransformFeedback) (void);
GL_APICALL void           (* GL_APIENTRY glResumeTransformFeedback) (void);
GL_APICALL void           (* GL_APIENTRY glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat,
        GLvoid *binary);
GL_APICALL void           (* GL_APIENTRY glProgramBinary) (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
GL_APICALL void           (* GL_APIENTRY glProgramParameteri) (GLuint program, GLenum pname, GLint value);
GL_APICALL void           (* GL_APIENTRY glInvalidateFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
GL_APICALL void           (* GL_APIENTRY glInvalidateSubFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum *attachments,
        GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void           (* GL_APIENTRY glTexStorage2D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
        GLsizei height);
GL_APICALL void           (* GL_APIENTRY glTexStorage3D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
        GLsizei height, GLsizei depth);
GL_APICALL void           (* GL_APIENTRY glGetInternalformativ) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize,
        GLint *params);

/* GL_ES_VERSION_3_1 */
GL_APICALL void           (* GL_APIENTRY glDispatchCompute) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
GL_APICALL void           (* GL_APIENTRY glDispatchComputeIndirect) (GLintptr indirect);
GL_APICALL void           (* GL_APIENTRY glDrawArraysIndirect) (GLenum mode, const void *indirect);
GL_APICALL void           (* GL_APIENTRY glDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect);
GL_APICALL void           (* GL_APIENTRY glFramebufferParameteri) (GLenum target, GLenum pname, GLint param);
GL_APICALL void           (* GL_APIENTRY glGetFramebufferParameteriv) (GLenum target, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetProgramInterfaceiv) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
GL_APICALL GLuint         (* GL_APIENTRY glGetProgramResourceIndex) (GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL void           (* GL_APIENTRY glGetProgramResourceName) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize,
        GLsizei *length, GLchar *name);
GL_APICALL void           (* GL_APIENTRY glGetProgramResourceiv) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount,
        const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL GLint          (* GL_APIENTRY glGetProgramResourceLocation) (GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL void           (* GL_APIENTRY glUseProgramStages) (GLuint pipeline, GLbitfield stages, GLuint program);
GL_APICALL void           (* GL_APIENTRY glActiveShaderProgram) (GLuint pipeline, GLuint program);
GL_APICALL GLuint         (* GL_APIENTRY glCreateShaderProgramv) (GLenum type, GLsizei count, const GLchar *const *strings);
GL_APICALL void           (* GL_APIENTRY glBindProgramPipeline) (GLuint pipeline);
GL_APICALL void           (* GL_APIENTRY glDeleteProgramPipelines) (GLsizei n, const GLuint *pipelines);
GL_APICALL void           (* GL_APIENTRY glGenProgramPipelines) (GLsizei n, GLuint *pipelines);
GL_APICALL GLboolean      (* GL_APIENTRY glIsProgramPipeline) (GLuint pipeline);
GL_APICALL void           (* GL_APIENTRY glGetProgramPipelineiv) (GLuint pipeline, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glProgramUniform1i) (GLuint program, GLint location, GLint v0);
GL_APICALL void           (* GL_APIENTRY glProgramUniform2i) (GLuint program, GLint location, GLint v0, GLint v1);
GL_APICALL void           (* GL_APIENTRY glProgramUniform3i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void           (* GL_APIENTRY glProgramUniform4i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void           (* GL_APIENTRY glProgramUniform1ui) (GLuint program, GLint location, GLuint v0);
GL_APICALL void           (* GL_APIENTRY glProgramUniform2ui) (GLuint program, GLint location, GLuint v0, GLuint v1);
GL_APICALL void           (* GL_APIENTRY glProgramUniform3ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void           (* GL_APIENTRY glProgramUniform4ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void           (* GL_APIENTRY glProgramUniform1f) (GLuint program, GLint location, GLfloat v0);
GL_APICALL void           (* GL_APIENTRY glProgramUniform2f) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void           (* GL_APIENTRY glProgramUniform3f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void           (* GL_APIENTRY glProgramUniform4f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2,
        GLfloat v3);
GL_APICALL void           (* GL_APIENTRY glProgramUniform1iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform2iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform3iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform4iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform1uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform2uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform3uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform4uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform1fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform2fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform3fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniform4fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix2x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix3x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix2x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix4x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix3x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glProgramUniformMatrix4x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose,
        const GLfloat *value);
GL_APICALL void           (* GL_APIENTRY glValidateProgramPipeline) (GLuint pipeline);
GL_APICALL void           (* GL_APIENTRY glGetProgramPipelineInfoLog) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void           (* GL_APIENTRY glBindImageTexture) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer,
        GLenum access, GLenum format);
GL_APICALL void           (* GL_APIENTRY glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
GL_APICALL void           (* GL_APIENTRY glMemoryBarrier) (GLbitfield barriers);
GL_APICALL void           (* GL_APIENTRY glMemoryBarrierByRegion) (GLbitfield barriers);
GL_APICALL void           (* GL_APIENTRY glTexStorage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
        GLsizei height, GLboolean fixedsamplelocations);
GL_APICALL void           (* GL_APIENTRY glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
GL_APICALL void           (* GL_APIENTRY glSampleMaski) (GLuint maskNumber, GLbitfield mask);
GL_APICALL void           (* GL_APIENTRY glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
GL_APICALL void           (* GL_APIENTRY glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
GL_APICALL void           (* GL_APIENTRY glVertexAttribFormat) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized,
        GLuint relativeoffset);
GL_APICALL void           (* GL_APIENTRY glVertexAttribIFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GL_APICALL void           (* GL_APIENTRY glVertexAttribBinding) (GLuint attribindex, GLuint bindingindex);
GL_APICALL void           (* GL_APIENTRY glVertexBindingDivisor) (GLuint bindingindex, GLuint divisor);

/* GL_ES_VERSION_3_2 */
GL_APICALL void           (* GL_APIENTRY glBlendBarrier) (void);
GL_APICALL void           (* GL_APIENTRY glCopyImageSubData) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY,
        GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight,
        GLsizei srcDepth);
GL_APICALL void           (* GL_APIENTRY glDebugMessageControl) (GLenum source, GLenum type, GLenum severity, GLsizei count,
        const GLuint *ids, GLboolean enabled);
GL_APICALL void           (* GL_APIENTRY glDebugMessageInsert) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
        const GLchar *buf);
GL_APICALL void           (* GL_APIENTRY glDebugMessageCallback) (GLDEBUGPROC callback, const void *userParam);
GL_APICALL GLuint         (* GL_APIENTRY glGetDebugMessageLog) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids,
        GLenum *severities, GLsizei *lengths, GLchar *messageLog);
GL_APICALL void           (* GL_APIENTRY glPushDebugGroup) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
GL_APICALL void           (* GL_APIENTRY glPopDebugGroup) (void);
GL_APICALL void           (* GL_APIENTRY glObjectLabel) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
GL_APICALL void           (* GL_APIENTRY glGetObjectLabel) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length,
        GLchar *label);
GL_APICALL void           (* GL_APIENTRY glObjectPtrLabel) (const void *ptr, GLsizei length, const GLchar *label);
GL_APICALL void           (* GL_APIENTRY glGetObjectPtrLabel) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
GL_APICALL void           (* GL_APIENTRY glGetPointerv) (GLenum pname, void **params);
GL_APICALL void           (* GL_APIENTRY glEnablei) (GLenum target, GLuint index);
GL_APICALL void           (* GL_APIENTRY glDisablei) (GLenum target, GLuint index);
GL_APICALL void           (* GL_APIENTRY glBlendEquationi) (GLuint buf, GLenum mode);
GL_APICALL void           (* GL_APIENTRY glBlendEquationSeparatei) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
GL_APICALL void           (* GL_APIENTRY glBlendFunci) (GLuint buf, GLenum src, GLenum dst);
GL_APICALL void           (* GL_APIENTRY glBlendFuncSeparatei) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
GL_APICALL void           (* GL_APIENTRY glColorMaski) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
GL_APICALL GLboolean      (* GL_APIENTRY glIsEnabledi) (GLenum target, GLuint index);
GL_APICALL void           (* GL_APIENTRY glDrawElementsBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices,
        GLint basevertex);
GL_APICALL void           (* GL_APIENTRY glDrawRangeElementsBaseVertex) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
        const void *indices, GLint basevertex);
GL_APICALL void           (* GL_APIENTRY glDrawElementsInstancedBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices,
        GLsizei instancecount, GLint basevertex);
GL_APICALL void           (* GL_APIENTRY glFramebufferTexture) (GLenum target, GLenum attachment, GLuint texture, GLint level);
GL_APICALL void           (* GL_APIENTRY glPrimitiveBoundingBox) (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX,
        GLfloat maxY, GLfloat maxZ, GLfloat maxW);
GL_APICALL GLenum         (* GL_APIENTRY glGetGraphicsResetStatus) (void);
GL_APICALL void           (* GL_APIENTRY glReadnPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
        GLsizei bufSize, void *data);
GL_APICALL void           (* GL_APIENTRY glGetnUniformfv) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
GL_APICALL void           (* GL_APIENTRY glGetnUniformiv) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetnUniformuiv) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
GL_APICALL void           (* GL_APIENTRY glMinSampleShading) (GLfloat value);
GL_APICALL void           (* GL_APIENTRY glPatchParameteri) (GLenum pname, GLint value);
GL_APICALL void           (* GL_APIENTRY glTexParameterIiv) (GLenum target, GLenum pname, const GLint *params);
GL_APICALL void           (* GL_APIENTRY glTexParameterIuiv) (GLenum target, GLenum pname, const GLuint *params);
GL_APICALL void           (* GL_APIENTRY glGetTexParameterIiv) (GLenum target, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetTexParameterIuiv) (GLenum target, GLenum pname, GLuint *params);
GL_APICALL void           (* GL_APIENTRY glSamplerParameterIiv) (GLuint sampler, GLenum pname, const GLint *param);
GL_APICALL void           (* GL_APIENTRY glSamplerParameterIuiv) (GLuint sampler, GLenum pname, const GLuint *param);
GL_APICALL void           (* GL_APIENTRY glGetSamplerParameterIiv) (GLuint sampler, GLenum pname, GLint *params);
GL_APICALL void           (* GL_APIENTRY glGetSamplerParameterIuiv) (GLuint sampler, GLenum pname, GLuint *params);
GL_APICALL void           (* GL_APIENTRY glTexBuffer) (GLenum target, GLenum internalformat, GLuint buffer);
GL_APICALL void           (* GL_APIENTRY glTexBufferRange) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset,
        GLsizeiptr size);
GL_APICALL void           (* GL_APIENTRY glTexStorage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
        GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
