/*
 * Copyright (C) 2015 The Android Open Source Project
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
#ifndef __gl3_h_
#define __gl3_h_

/*
 * stub gl3.h for dynamic loading, based on:
 * gl3.h last updated on $Date: 2013-02-12 14:37:24 -0800 (Tue, 12 Feb 2013) $
 *
 * Changes:
 * - Added #include <GLES2/gl2.h>
 * - Removed duplicate OpenGL ES 2.0 declarations
 * - Converted OpenGL ES 3.0 function prototypes to function pointer
 *   declarations
 * - Added gl3stubInit() declaration
 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/api-level.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright (c) 2007-2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

/* Call this function before calling any OpenGL ES 3.0 functions. It will
 * return GL_TRUE if the OpenGL ES 3.0 was successfully initialized, GL_FALSE
 * otherwise. */
#ifndef GL_ES_VERSION_2_0
#define GL_ES_VERSION_2_0  2
#endif
#define GL_ES_VERSION_3_0  3
#define GL_ES_VERSION_3_1  4
#define GL_ES_VERSION_3_2  5

GLboolean gl3stubInit();

/*-------------------------------------------------------------------------
 * Data type definitions
 *-----------------------------------------------------------------------*/

/* OpenGL ES 3.0 */

typedef unsigned short   GLhalf;
#if __ANDROID_API__ <= 19
typedef khronos_int64_t  GLint64;
typedef khronos_uint64_t GLuint64;
typedef struct __GLsync *GLsync;
#endif

/*-------------------------------------------------------------------------
 * Token definitions
 *-----------------------------------------------------------------------*/

/* OpenGL ES 3.0 */

#define GL_READ_BUFFER                                   0x0C02
#define GL_UNPACK_ROW_LENGTH                             0x0CF2
#define GL_UNPACK_SKIP_ROWS                              0x0CF3
#define GL_UNPACK_SKIP_PIXELS                            0x0CF4
#define GL_PACK_ROW_LENGTH                               0x0D02
#define GL_PACK_SKIP_ROWS                                0x0D03
#define GL_PACK_SKIP_PIXELS                              0x0D04
#define GL_COLOR                                         0x1800
#define GL_DEPTH                                         0x1801
#define GL_STENCIL                                       0x1802
#define GL_RED                                           0x1903
#define GL_RGB8                                          0x8051
#define GL_RGBA8                                         0x8058
#define GL_RGB10_A2                                      0x8059
#define GL_TEXTURE_BINDING_3D                            0x806A
#define GL_UNPACK_SKIP_IMAGES                            0x806D
#define GL_UNPACK_IMAGE_HEIGHT                           0x806E
#define GL_TEXTURE_3D                                    0x806F
#define GL_TEXTURE_WRAP_R                                0x8072
#define GL_MAX_3D_TEXTURE_SIZE                           0x8073
#define GL_UNSIGNED_INT_2_10_10_10_REV                   0x8368
#define GL_MAX_ELEMENTS_VERTICES                         0x80E8
#define GL_MAX_ELEMENTS_INDICES                          0x80E9
#define GL_TEXTURE_MIN_LOD                               0x813A
#define GL_TEXTURE_MAX_LOD                               0x813B
#define GL_TEXTURE_BASE_LEVEL                            0x813C
#define GL_TEXTURE_MAX_LEVEL                             0x813D
#define GL_MIN                                           0x8007
#define GL_MAX                                           0x8008
#define GL_DEPTH_COMPONENT24                             0x81A6
#define GL_MAX_TEXTURE_LOD_BIAS                          0x84FD
#define GL_TEXTURE_COMPARE_MODE                          0x884C
#define GL_TEXTURE_COMPARE_FUNC                          0x884D
#define GL_CURRENT_QUERY                                 0x8865
#define GL_QUERY_RESULT                                  0x8866
#define GL_QUERY_RESULT_AVAILABLE                        0x8867
#define GL_BUFFER_MAPPED                                 0x88BC
#define GL_BUFFER_MAP_POINTER                            0x88BD
#define GL_STREAM_READ                                   0x88E1
#define GL_STREAM_COPY                                   0x88E2
#define GL_STATIC_READ                                   0x88E5
#define GL_STATIC_COPY                                   0x88E6
#define GL_DYNAMIC_READ                                  0x88E9
#define GL_DYNAMIC_COPY                                  0x88EA
#define GL_MAX_DRAW_BUFFERS                              0x8824
#define GL_DRAW_BUFFER0                                  0x8825
#define GL_DRAW_BUFFER1                                  0x8826
#define GL_DRAW_BUFFER2                                  0x8827
#define GL_DRAW_BUFFER3                                  0x8828
#define GL_DRAW_BUFFER4                                  0x8829
#define GL_DRAW_BUFFER5                                  0x882A
#define GL_DRAW_BUFFER6                                  0x882B
#define GL_DRAW_BUFFER7                                  0x882C
#define GL_DRAW_BUFFER8                                  0x882D
#define GL_DRAW_BUFFER9                                  0x882E
#define GL_DRAW_BUFFER10                                 0x882F
#define GL_DRAW_BUFFER11                                 0x8830
#define GL_DRAW_BUFFER12                                 0x8831
#define GL_DRAW_BUFFER13                                 0x8832
#define GL_DRAW_BUFFER14                                 0x8833
#define GL_DRAW_BUFFER15                                 0x8834
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS               0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                 0x8B4A
#define GL_SAMPLER_3D                                    0x8B5F
#define GL_SAMPLER_2D_SHADOW                             0x8B62
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT               0x8B8B
#define GL_PIXEL_PACK_BUFFER                             0x88EB
#define GL_PIXEL_UNPACK_BUFFER                           0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING                     0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING                   0x88EF
#define GL_FLOAT_MAT2x3                                  0x8B65
#define GL_FLOAT_MAT2x4                                  0x8B66
#define GL_FLOAT_MAT3x2                                  0x8B67
#define GL_FLOAT_MAT3x4                                  0x8B68
#define GL_FLOAT_MAT4x2                                  0x8B69
#define GL_FLOAT_MAT4x3                                  0x8B6A
#define GL_SRGB                                          0x8C40
#define GL_SRGB8                                         0x8C41
#define GL_SRGB8_ALPHA8                                  0x8C43
#define GL_COMPARE_REF_TO_TEXTURE                        0x884E
#define GL_MAJOR_VERSION                                 0x821B
#define GL_MINOR_VERSION                                 0x821C
#define GL_NUM_EXTENSIONS                                0x821D
#define GL_RGBA32F                                       0x8814
#define GL_RGB32F                                        0x8815
#define GL_RGBA16F                                       0x881A
#define GL_RGB16F                                        0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                   0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS                      0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET                      0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET                      0x8905
#define GL_MAX_VARYING_COMPONENTS                        0x8B4B
#define GL_TEXTURE_2D_ARRAY                              0x8C1A
#define GL_TEXTURE_BINDING_2D_ARRAY                      0x8C1D
#define GL_R11F_G11F_B10F                                0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV                  0x8C3B
#define GL_RGB9_E5                                       0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV                      0x8C3E
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH         0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE                0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS                   0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START               0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE                0x8C85
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN         0x8C88
#define GL_RASTERIZER_DISCARD                            0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       0x8C8B
#define GL_INTERLEAVED_ATTRIBS                           0x8C8C
#define GL_SEPARATE_ATTRIBS                              0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER                     0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING             0x8C8F
#define GL_RGBA32UI                                      0x8D70
#define GL_RGB32UI                                       0x8D71
#define GL_RGBA16UI                                      0x8D76
#define GL_RGB16UI                                       0x8D77
#define GL_RGBA8UI                                       0x8D7C
#define GL_RGB8UI                                        0x8D7D
#define GL_RGBA32I                                       0x8D82
#define GL_RGB32I                                        0x8D83
#define GL_RGBA16I                                       0x8D88
#define GL_RGB16I                                        0x8D89
#define GL_RGBA8I                                        0x8D8E
#define GL_RGB8I                                         0x8D8F
#define GL_RED_INTEGER                                   0x8D94
#define GL_RGB_INTEGER                                   0x8D98
#define GL_RGBA_INTEGER                                  0x8D99
#define GL_SAMPLER_2D_ARRAY                              0x8DC1
#define GL_SAMPLER_2D_ARRAY_SHADOW                       0x8DC4
#define GL_SAMPLER_CUBE_SHADOW                           0x8DC5
#define GL_UNSIGNED_INT_VEC2                             0x8DC6
#define GL_UNSIGNED_INT_VEC3                             0x8DC7
#define GL_UNSIGNED_INT_VEC4                             0x8DC8
#define GL_INT_SAMPLER_2D                                0x8DCA
#define GL_INT_SAMPLER_3D                                0x8DCB
#define GL_INT_SAMPLER_CUBE                              0x8DCC
#define GL_INT_SAMPLER_2D_ARRAY                          0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_2D                       0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D                       0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE                     0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                 0x8DD7
#define GL_BUFFER_ACCESS_FLAGS                           0x911F
#define GL_BUFFER_MAP_LENGTH                             0x9120
#define GL_BUFFER_MAP_OFFSET                             0x9121
#define GL_DEPTH_COMPONENT32F                            0x8CAC
#define GL_DEPTH32F_STENCIL8                             0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                0x8DAD
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING         0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE         0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE               0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE             0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE              0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE             0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE             0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE           0x8217
#define GL_FRAMEBUFFER_DEFAULT                           0x8218
#define GL_FRAMEBUFFER_UNDEFINED                         0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT                      0x821A
#define GL_DEPTH_STENCIL                                 0x84F9
#define GL_UNSIGNED_INT_24_8                             0x84FA
#define GL_DEPTH24_STENCIL8                              0x88F0
#define GL_UNSIGNED_NORMALIZED                           0x8C17
#define GL_DRAW_FRAMEBUFFER_BINDING                      GL_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER                              0x8CA8
#define GL_DRAW_FRAMEBUFFER                              0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING                      0x8CAA
#define GL_RENDERBUFFER_SAMPLES                          0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER          0x8CD4
#define GL_MAX_COLOR_ATTACHMENTS                         0x8CDF
#define GL_COLOR_ATTACHMENT1                             0x8CE1
#define GL_COLOR_ATTACHMENT2                             0x8CE2
#define GL_COLOR_ATTACHMENT3                             0x8CE3
#define GL_COLOR_ATTACHMENT4                             0x8CE4
#define GL_COLOR_ATTACHMENT5                             0x8CE5
#define GL_COLOR_ATTACHMENT6                             0x8CE6
#define GL_COLOR_ATTACHMENT7                             0x8CE7
#define GL_COLOR_ATTACHMENT8                             0x8CE8
#define GL_COLOR_ATTACHMENT9                             0x8CE9
#define GL_COLOR_ATTACHMENT10                            0x8CEA
#define GL_COLOR_ATTACHMENT11                            0x8CEB
#define GL_COLOR_ATTACHMENT12                            0x8CEC
#define GL_COLOR_ATTACHMENT13                            0x8CED
#define GL_COLOR_ATTACHMENT14                            0x8CEE
#define GL_COLOR_ATTACHMENT15                            0x8CEF
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE            0x8D56
#define GL_MAX_SAMPLES                                   0x8D57
#define GL_HALF_FLOAT                                    0x140B
#define GL_MAP_READ_BIT                                  0x0001
#define GL_MAP_WRITE_BIT                                 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                      0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                     0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT                        0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT                        0x0020
#define GL_RG                                            0x8227
#define GL_RG_INTEGER                                    0x8228
#define GL_R8                                            0x8229
#define GL_RG8                                           0x822B
#define GL_R16F                                          0x822D
#define GL_R32F                                          0x822E
#define GL_RG16F                                         0x822F
#define GL_RG32F                                         0x8230
#define GL_R8I                                           0x8231
#define GL_R8UI                                          0x8232
#define GL_R16I                                          0x8233
#define GL_R16UI                                         0x8234
#define GL_R32I                                          0x8235
#define GL_R32UI                                         0x8236
#define GL_RG8I                                          0x8237
#define GL_RG8UI                                         0x8238
#define GL_RG16I                                         0x8239
#define GL_RG16UI                                        0x823A
#define GL_RG32I                                         0x823B
#define GL_RG32UI                                        0x823C
#define GL_VERTEX_ARRAY_BINDING                          0x85B5
#define GL_R8_SNORM                                      0x8F94
#define GL_RG8_SNORM                                     0x8F95
#define GL_RGB8_SNORM                                    0x8F96
#define GL_RGBA8_SNORM                                   0x8F97
#define GL_SIGNED_NORMALIZED                             0x8F9C
#define GL_PRIMITIVE_RESTART_FIXED_INDEX                 0x8D69
#define GL_COPY_READ_BUFFER                              0x8F36
#define GL_COPY_WRITE_BUFFER                             0x8F37
#define GL_COPY_READ_BUFFER_BINDING                      GL_COPY_READ_BUFFER
#define GL_COPY_WRITE_BUFFER_BINDING                     GL_COPY_WRITE_BUFFER
#define GL_UNIFORM_BUFFER                                0x8A11
#define GL_UNIFORM_BUFFER_BINDING                        0x8A28
#define GL_UNIFORM_BUFFER_START                          0x8A29
#define GL_UNIFORM_BUFFER_SIZE                           0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                     0x8A2B
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                   0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                   0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                        0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        0x8A31
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT               0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH          0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS                         0x8A36
#define GL_UNIFORM_TYPE                                  0x8A37
#define GL_UNIFORM_SIZE                                  0x8A38
#define GL_UNIFORM_NAME_LENGTH                           0x8A39
#define GL_UNIFORM_BLOCK_INDEX                           0x8A3A
#define GL_UNIFORM_OFFSET                                0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                          0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                         0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                          0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                         0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                       0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH                     0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                 0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES          0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER     0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER   0x8A46
#define GL_INVALID_INDEX                                 0xFFFFFFFFu
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                  0x9122
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
#define GL_MAX_SERVER_WAIT_TIMEOUT                       0x9111
#define GL_OBJECT_TYPE                                   0x9112
#define GL_SYNC_CONDITION                                0x9113
#define GL_SYNC_STATUS                                   0x9114
#define GL_SYNC_FLAGS                                    0x9115
#define GL_SYNC_FENCE                                    0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE                    0x9117
#define GL_UNSIGNALED                                    0x9118
#define GL_SIGNALED                                      0x9119
#define GL_ALREADY_SIGNALED                              0x911A
#define GL_TIMEOUT_EXPIRED                               0x911B
#define GL_CONDITION_SATISFIED                           0x911C
#define GL_WAIT_FAILED                                   0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT                       0x00000001
#define GL_TIMEOUT_IGNORED                               0xFFFFFFFFFFFFFFFFull
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR                   0x88FE
#define GL_ANY_SAMPLES_PASSED                            0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE               0x8D6A
#define GL_SAMPLER_BINDING                               0x8919
#define GL_RGB10_A2UI                                    0x906F
#define GL_TEXTURE_SWIZZLE_R                             0x8E42
#define GL_TEXTURE_SWIZZLE_G                             0x8E43
#define GL_TEXTURE_SWIZZLE_B                             0x8E44
#define GL_TEXTURE_SWIZZLE_A                             0x8E45
#define GL_GREEN                                         0x1904
#define GL_BLUE                                          0x1905
#define GL_INT_2_10_10_10_REV                            0x8D9F
#define GL_TRANSFORM_FEEDBACK                            0x8E22
#define GL_TRANSFORM_FEEDBACK_PAUSED                     0x8E23
#define GL_TRANSFORM_FEEDBACK_ACTIVE                     0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING                    0x8E25
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT               0x8257
#define GL_PROGRAM_BINARY_LENGTH                         0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS                    0x87FE
#define GL_PROGRAM_BINARY_FORMATS                        0x87FF
#define GL_COMPRESSED_R11_EAC                            0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
#define GL_COMPRESSED_RG11_EAC                           0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
#define GL_COMPRESSED_RGB8_ETC2                          0x9274
#define GL_COMPRESSED_SRGB8_ETC2                         0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279
#define GL_TEXTURE_IMMUTABLE_FORMAT                      0x912F
#define GL_MAX_ELEMENT_INDEX                             0x8D6B
#define GL_NUM_SAMPLE_COUNTS                             0x9380
#define GL_TEXTURE_IMMUTABLE_LEVELS                      0x82DF

/*-------------------------------------------------------------------------
 * Entrypoint definitions
 *-----------------------------------------------------------------------*/

/* OpenGL ES 3.0 */

extern GL_APICALL void           (* GL_APIENTRY glReadBuffer) (GLenum mode);
extern GL_APICALL void           (* GL_APIENTRY glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
extern GL_APICALL void           (* GL_APIENTRY glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
extern GL_APICALL void           (* GL_APIENTRY glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
extern GL_APICALL void           (* GL_APIENTRY glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern GL_APICALL void           (* GL_APIENTRY glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
extern GL_APICALL void           (* GL_APIENTRY glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
extern GL_APICALL void           (* GL_APIENTRY glGenQueries) (GLsizei n, GLuint* ids);
extern GL_APICALL void           (* GL_APIENTRY glDeleteQueries) (GLsizei n, const GLuint* ids);
extern GL_APICALL GLboolean      (* GL_APIENTRY glIsQuery) (GLuint id);
extern GL_APICALL void           (* GL_APIENTRY glBeginQuery) (GLenum target, GLuint id);
extern GL_APICALL void           (* GL_APIENTRY glEndQuery) (GLenum target);
extern GL_APICALL void           (* GL_APIENTRY glGetQueryiv) (GLenum target, GLenum pname, GLint* params);
extern GL_APICALL void           (* GL_APIENTRY glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint* params);
extern GL_APICALL GLboolean      (* GL_APIENTRY glUnmapBuffer) (GLenum target);
extern GL_APICALL void           (* GL_APIENTRY glGetBufferPointerv) (GLenum target, GLenum pname, GLvoid** params);
extern GL_APICALL void           (* GL_APIENTRY glDrawBuffers) (GLsizei n, const GLenum* bufs);
extern GL_APICALL void           (* GL_APIENTRY glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern GL_APICALL void           (* GL_APIENTRY glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern GL_APICALL void           (* GL_APIENTRY glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern GL_APICALL GLvoid*        (* GL_APIENTRY glMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern GL_APICALL void           (* GL_APIENTRY glFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
extern GL_APICALL void           (* GL_APIENTRY glBindVertexArray) (GLuint array);
extern GL_APICALL void           (* GL_APIENTRY glDeleteVertexArrays) (GLsizei n, const GLuint* arrays);
extern GL_APICALL void           (* GL_APIENTRY glGenVertexArrays) (GLsizei n, GLuint* arrays);
extern GL_APICALL GLboolean      (* GL_APIENTRY glIsVertexArray) (GLuint array);
extern GL_APICALL void           (* GL_APIENTRY glGetIntegeri_v) (GLenum target, GLuint index, GLint* data);
extern GL_APICALL void           (* GL_APIENTRY glBeginTransformFeedback) (GLenum primitiveMode);
extern GL_APICALL void           (* GL_APIENTRY glEndTransformFeedback) (void);
extern GL_APICALL void           (* GL_APIENTRY glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern GL_APICALL void           (* GL_APIENTRY glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
extern GL_APICALL void           (* GL_APIENTRY glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
extern GL_APICALL void           (* GL_APIENTRY glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
extern GL_APICALL void           (* GL_APIENTRY glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
extern GL_APICALL void           (* GL_APIENTRY glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint* params);
extern GL_APICALL void           (* GL_APIENTRY glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint* params);
extern GL_APICALL void           (* GL_APIENTRY glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
extern GL_APICALL void           (* GL_APIENTRY glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern GL_APICALL void           (* GL_APIENTRY glVertexAttribI4iv) (GLuint index, const GLint* v);
extern GL_APICALL void           (* GL_APIENTRY glVertexAttribI4uiv) (GLuint index, const GLuint* v);
extern GL_APICALL void           (* GL_APIENTRY glGetUniformuiv) (GLuint program, GLint location, GLuint* params);
extern GL_APICALL GLint          (* GL_APIENTRY glGetFragDataLocation) (GLuint program, const GLchar *name);
extern GL_APICALL void           (* GL_APIENTRY glUniform1ui) (GLint location, GLuint v0);
extern GL_APICALL void           (* GL_APIENTRY glUniform2ui) (GLint location, GLuint v0, GLuint v1);
extern GL_APICALL void           (* GL_APIENTRY glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GL_APICALL void           (* GL_APIENTRY glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern GL_APICALL void           (* GL_APIENTRY glUniform1uiv) (GLint location, GLsizei count, const GLuint* value);
extern GL_APICALL void           (* GL_APIENTRY glUniform2uiv) (GLint location, GLsizei count, const GLuint* value);
extern GL_APICALL void           (* GL_APIENTRY glUniform3uiv) (GLint location, GLsizei count, const GLuint* value);
extern GL_APICALL void           (* GL_APIENTRY glUniform4uiv) (GLint location, GLsizei count, const GLuint* value);
extern GL_APICALL void           (* GL_APIENTRY glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint* value);
extern GL_APICALL void           (* GL_APIENTRY glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint* value);
extern GL_APICALL void           (* GL_APIENTRY glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat* value);
extern GL_APICALL void           (* GL_APIENTRY glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
extern GL_APICALL const GLubyte* (* GL_APIENTRY glGetStringi) (GLenum name, GLuint index);
extern GL_APICALL void           (* GL_APIENTRY glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern GL_APICALL void           (* GL_APIENTRY glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
extern GL_APICALL void           (* GL_APIENTRY glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
extern GL_APICALL GLuint         (* GL_APIENTRY glGetUniformBlockIndex) (GLuint program, const GLchar* uniformBlockName);
extern GL_APICALL void           (* GL_APIENTRY glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
extern GL_APICALL void           (* GL_APIENTRY glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
extern GL_APICALL void           (* GL_APIENTRY glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
extern GL_APICALL void           (* GL_APIENTRY glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
extern GL_APICALL void           (* GL_APIENTRY glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
extern GL_APICALL GLsync         (* GL_APIENTRY glFenceSync) (GLenum condition, GLbitfield flags);
extern GL_APICALL GLboolean      (* GL_APIENTRY glIsSync) (GLsync sync);
extern GL_APICALL void           (* GL_APIENTRY glDeleteSync) (GLsync sync);
extern GL_APICALL GLenum         (* GL_APIENTRY glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern GL_APICALL void           (* GL_APIENTRY glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
extern GL_APICALL void           (* GL_APIENTRY glGetInteger64v) (GLenum pname, GLint64* params);
extern GL_APICALL void           (* GL_APIENTRY glGetSynciv) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
extern GL_APICALL void           (* GL_APIENTRY glGetInteger64i_v) (GLenum target, GLuint index, GLint64* data);
extern GL_APICALL void           (* GL_APIENTRY glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64* params);
extern GL_APICALL void           (* GL_APIENTRY glGenSamplers) (GLsizei count, GLuint* samplers);
extern GL_APICALL void           (* GL_APIENTRY glDeleteSamplers) (GLsizei count, const GLuint* samplers);
extern GL_APICALL GLboolean      (* GL_APIENTRY glIsSampler) (GLuint sampler);
extern GL_APICALL void           (* GL_APIENTRY glBindSampler) (GLuint unit, GLuint sampler);
extern GL_APICALL void           (* GL_APIENTRY glSamplerParameteri) (GLuint sampler, GLenum pname, GLint param);
extern GL_APICALL void           (* GL_APIENTRY glSamplerParameteriv) (GLuint sampler, GLenum pname, const GLint* param);
extern GL_APICALL void           (* GL_APIENTRY glSamplerParameterf) (GLuint sampler, GLenum pname, GLfloat param);
extern GL_APICALL void           (* GL_APIENTRY glSamplerParameterfv) (GLuint sampler, GLenum pname, const GLfloat* param);
extern GL_APICALL void           (* GL_APIENTRY glGetSamplerParameteriv) (GLuint sampler, GLenum pname, GLint* params);
extern GL_APICALL void           (* GL_APIENTRY glGetSamplerParameterfv) (GLuint sampler, GLenum pname, GLfloat* params);
extern GL_APICALL void           (* GL_APIENTRY glVertexAttribDivisor) (GLuint index, GLuint divisor);
extern GL_APICALL void           (* GL_APIENTRY glBindTransformFeedback) (GLenum target, GLuint id);
extern GL_APICALL void           (* GL_APIENTRY glDeleteTransformFeedbacks) (GLsizei n, const GLuint* ids);
extern GL_APICALL void           (* GL_APIENTRY glGenTransformFeedbacks) (GLsizei n, GLuint* ids);
extern GL_APICALL GLboolean      (* GL_APIENTRY glIsTransformFeedback) (GLuint id);
extern GL_APICALL void           (* GL_APIENTRY glPauseTransformFeedback) (void);
extern GL_APICALL void           (* GL_APIENTRY glResumeTransformFeedback) (void);
extern GL_APICALL void           (* GL_APIENTRY glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
extern GL_APICALL void           (* GL_APIENTRY glProgramBinary) (GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
extern GL_APICALL void           (* GL_APIENTRY glProgramParameteri) (GLuint program, GLenum pname, GLint value);
extern GL_APICALL void           (* GL_APIENTRY glInvalidateFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
extern GL_APICALL void           (* GL_APIENTRY glInvalidateSubFramebuffer) (GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
extern GL_APICALL void           (* GL_APIENTRY glTexStorage2D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern GL_APICALL void           (* GL_APIENTRY glTexStorage3D) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern GL_APICALL void           (* GL_APIENTRY glGetInternalformativ) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

#define GL_COMPUTE_SHADER                 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS     0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS     0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS    0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT   0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE    0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE        0x8267
#define GL_DISPATCH_INDIRECT_BUFFER       0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#define GL_COMPUTE_SHADER_BIT             0x00000020
#define GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#define GL_MAX_UNIFORM_LOCATIONS          0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH      0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT     0x9311
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES    0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH          0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT         0x9316
#define GL_MAX_FRAMEBUFFER_SAMPLES        0x9318
#define GL_UNIFORM                        0x92E1
#define GL_UNIFORM_BLOCK                  0x92E2
#define GL_PROGRAM_INPUT                  0x92E3
#define GL_PROGRAM_OUTPUT                 0x92E4
#define GL_BUFFER_VARIABLE                0x92E5
#define GL_SHADER_STORAGE_BLOCK           0x92E6
#define GL_ATOMIC_COUNTER_BUFFER          0x92C0
#define GL_TRANSFORM_FEEDBACK_VARYING     0x92F4
#define GL_ACTIVE_RESOURCES               0x92F5
#define GL_MAX_NAME_LENGTH                0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES       0x92F7
#define GL_NAME_LENGTH                    0x92F9
#define GL_TYPE                           0x92FA
#define GL_ARRAY_SIZE                     0x92FB
#define GL_OFFSET                         0x92FC
#define GL_BLOCK_INDEX                    0x92FD
#define GL_ARRAY_STRIDE                   0x92FE
#define GL_MATRIX_STRIDE                  0x92FF
#define GL_IS_ROW_MAJOR                   0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX    0x9301
#define GL_BUFFER_BINDING                 0x9302
#define GL_BUFFER_DATA_SIZE               0x9303
#define GL_NUM_ACTIVE_VARIABLES           0x9304
#define GL_ACTIVE_VARIABLES               0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER    0x9306
#define GL_REFERENCED_BY_FRAGMENT_SHADER  0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER   0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE           0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE         0x930D
#define GL_LOCATION                       0x930E
#define GL_VERTEX_SHADER_BIT              0x00000001
#define GL_FRAGMENT_SHADER_BIT            0x00000002
#define GL_ALL_SHADER_BITS                0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE              0x8258
#define GL_ACTIVE_PROGRAM                 0x8259
#define GL_PROGRAM_PIPELINE_BINDING       0x825A
#define GL_ATOMIC_COUNTER_BUFFER_BINDING  0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START    0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE     0x92C3
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS     0x92D2
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS   0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS   0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS  0x92D9
#define GL_UNSIGNED_INT_ATOMIC_COUNTER    0x92DB
#define GL_MAX_IMAGE_UNITS                0x8F38
#define GL_MAX_VERTEX_IMAGE_UNIFORMS      0x90CA
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS    0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS    0x90CF
#define GL_IMAGE_BINDING_NAME             0x8F3A
#define GL_IMAGE_BINDING_LEVEL            0x8F3B
#define GL_IMAGE_BINDING_LAYERED          0x8F3C
#define GL_IMAGE_BINDING_LAYER            0x8F3D
#define GL_IMAGE_BINDING_ACCESS           0x8F3E
#define GL_IMAGE_BINDING_FORMAT           0x906E
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT      0x00000002
#define GL_UNIFORM_BARRIER_BIT            0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT      0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT            0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT       0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT     0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT      0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT        0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT     0x00001000
#define GL_ALL_BARRIER_BITS               0xFFFFFFFF
#define GL_IMAGE_2D                       0x904D
#define GL_IMAGE_3D                       0x904E
#define GL_IMAGE_CUBE                     0x9050
#define GL_IMAGE_2D_ARRAY                 0x9053
#define GL_INT_IMAGE_2D                   0x9058
#define GL_INT_IMAGE_3D                   0x9059
#define GL_INT_IMAGE_CUBE                 0x905B
#define GL_INT_IMAGE_2D_ARRAY             0x905E
#define GL_UNSIGNED_INT_IMAGE_2D          0x9063
#define GL_UNSIGNED_INT_IMAGE_3D          0x9064
#define GL_UNSIGNED_INT_IMAGE_CUBE        0x9066
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY    0x9069
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE  0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT     0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE     0x90EA
#define GL_STENCIL_INDEX                  0x1901
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_SAMPLE_POSITION                0x8E50
#define GL_SAMPLE_MASK                    0x8E51
#define GL_SAMPLE_MASK_VALUE              0x8E52
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_MAX_SAMPLE_MASK_WORDS          0x8E59
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
#define GL_MAX_INTEGER_SAMPLES            0x9110
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_SAMPLES                0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_TEXTURE_STENCIL_SIZE           0x88F1
#define GL_TEXTURE_SHARED_SIZE            0x8C3F
#define GL_TEXTURE_RED_TYPE               0x8C10
#define GL_TEXTURE_GREEN_TYPE             0x8C11
#define GL_TEXTURE_BLUE_TYPE              0x8C12
#define GL_TEXTURE_ALPHA_TYPE             0x8C13
#define GL_TEXTURE_DEPTH_TYPE             0x8C16
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_VERTEX_ATTRIB_BINDING          0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET  0x82D5
#define GL_VERTEX_BINDING_DIVISOR         0x82D6
#define GL_VERTEX_BINDING_OFFSET          0x82D7
#define GL_VERTEX_BINDING_STRIDE          0x82D8
#define GL_VERTEX_BINDING_BUFFER          0x8F4F
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS     0x82DA
#define GL_MAX_VERTEX_ATTRIB_STRIDE       0x82E5
/* GL_ES_VERSION_3_1 */
extern GL_APICALL void ( *GL_APIENTRY glDispatchCompute) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
extern GL_APICALL void ( *GL_APIENTRY glDispatchComputeIndirect) (GLintptr indirect);
extern GL_APICALL void ( *GL_APIENTRY glDrawArraysIndirect) (GLenum mode, const void *indirect);
extern GL_APICALL void ( *GL_APIENTRY glDrawElementsIndirect) (GLenum mode, GLenum type, const void *indirect);
extern GL_APICALL void ( *GL_APIENTRY glFramebufferParameteri) (GLenum target, GLenum pname, GLint param);
extern GL_APICALL void ( *GL_APIENTRY glGetFramebufferParameteriv) (GLenum target, GLenum pname, GLint *params);
extern GL_APICALL void ( *GL_APIENTRY glGetProgramInterfaceiv) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
extern GL_APICALL GLuint ( *GL_APIENTRY glGetProgramResourceIndex) (GLuint program, GLenum programInterface, const GLchar *name);
extern GL_APICALL void ( *GL_APIENTRY glGetProgramResourceName) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern GL_APICALL void ( * GL_APIENTRY glGetProgramResourceiv) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
extern GL_APICALL GLint (* GL_APIENTRY glGetProgramResourceLocation) (GLuint program, GLenum programInterface, const GLchar *name);
extern GL_APICALL void (* GL_APIENTRY glUseProgramStages) (GLuint pipeline, GLbitfield stages, GLuint program);
extern GL_APICALL void (* GL_APIENTRY glActiveShaderProgram) (GLuint pipeline, GLuint program);
extern GL_APICALL GLuint (* GL_APIENTRY glCreateShaderProgramv) (GLenum type, GLsizei count, const GLchar *const*strings);
extern GL_APICALL void (* GL_APIENTRY glBindProgramPipeline) (GLuint pipeline);
extern GL_APICALL void (* GL_APIENTRY glDeleteProgramPipelines) (GLsizei n, const GLuint *pipelines);
extern GL_APICALL void (* GL_APIENTRY glGenProgramPipelines) (GLsizei n, GLuint *pipelines);
extern GL_APICALL GLboolean (* GL_APIENTRY glIsProgramPipeline) (GLuint pipeline);
extern GL_APICALL void (* GL_APIENTRY glGetProgramPipelineiv) (GLuint pipeline, GLenum pname, GLint *params);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform1i) (GLuint program, GLint location, GLint v0);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform2i) (GLuint program, GLint location, GLint v0, GLint v1);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform3i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform4i) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform1ui) (GLuint program, GLint location, GLuint v0);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform2ui) (GLuint program, GLint location, GLuint v0, GLuint v1);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform3ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform4ui) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform1f) (GLuint program, GLint location, GLfloat v0);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform2f) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform3f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform4f) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform1iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform2iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform3iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform4iv) (GLuint program, GLint location, GLsizei count, const GLint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform1uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform2uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform3uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform4uiv) (GLuint program, GLint location, GLsizei count, const GLuint *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform1fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform2fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform3fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniform4fv) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix2x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix3x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix2x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix4x2fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix3x4fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glProgramUniformMatrix4x3fv) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GL_APICALL void (* GL_APIENTRY glValidateProgramPipeline) (GLuint pipeline);
extern GL_APICALL void (* GL_APIENTRY glGetProgramPipelineInfoLog) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern GL_APICALL void (* GL_APIENTRY glBindImageTexture) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
extern GL_APICALL void (* GL_APIENTRY glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
extern GL_APICALL void (* GL_APIENTRY glMemoryBarrier) (GLbitfield barriers);
extern GL_APICALL void (* GL_APIENTRY glMemoryBarrierByRegion) (GLbitfield barriers);
extern GL_APICALL void (* GL_APIENTRY glTexStorage2DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
extern GL_APICALL void (* GL_APIENTRY glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
extern GL_APICALL void (* GL_APIENTRY glSampleMaski) (GLuint maskNumber, GLbitfield mask);
extern GL_APICALL void (* GL_APIENTRY glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
extern GL_APICALL void (* GL_APIENTRY glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
extern GL_APICALL void (* GL_APIENTRY glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
extern GL_APICALL void (* GL_APIENTRY glVertexAttribFormat) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
extern GL_APICALL void (* GL_APIENTRY glVertexAttribIFormat) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
extern GL_APICALL void (* GL_APIENTRY glVertexAttribBinding) (GLuint attribindex, GLuint bindingindex);
extern GL_APICALL void (* GL_APIENTRY glVertexBindingDivisor) (GLuint bindingindex, GLuint divisor);

typedef void (GL_APIENTRY  *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
#define GL_MULTISAMPLE_LINE_WIDTH_RANGE   0x9381
#define GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY 0x9382
#define GL_MULTIPLY                       0x9294
#define GL_SCREEN                         0x9295
#define GL_OVERLAY                        0x9296
#define GL_DARKEN                         0x9297
#define GL_LIGHTEN                        0x9298
#define GL_COLORDODGE                     0x9299
#define GL_COLORBURN                      0x929A
#define GL_HARDLIGHT                      0x929B
#define GL_SOFTLIGHT                      0x929C
#define GL_DIFFERENCE                     0x929E
#define GL_EXCLUSION                      0x92A0
#define GL_HSL_HUE                        0x92AD
#define GL_HSL_SATURATION                 0x92AE
#define GL_HSL_COLOR                      0x92AF
#define GL_HSL_LUMINOSITY                 0x92B0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION        0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM      0x8245
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#define GL_DEBUG_SOURCE_OTHER             0x824B
#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#define GL_DEBUG_TYPE_OTHER               0x8251
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH    0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH        0x826D
#define GL_BUFFER                         0x82E0
#define GL_SHADER                         0x82E1
#define GL_PROGRAM                        0x82E2
#define GL_VERTEX_ARRAY                   0x8074
#define GL_QUERY                          0x82E3
#define GL_PROGRAM_PIPELINE               0x82E4
#define GL_SAMPLER                        0x82E6
#define GL_MAX_LABEL_LENGTH               0x82E8
#define GL_MAX_DEBUG_MESSAGE_LENGTH       0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES      0x9144
#define GL_DEBUG_LOGGED_MESSAGES          0x9145
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT         0x00000002
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_GEOMETRY_SHADER                0x8DD9
#define GL_GEOMETRY_SHADER_BIT            0x00000004
#define GL_GEOMETRY_VERTICES_OUT          0x8916
#define GL_GEOMETRY_INPUT_TYPE            0x8917
#define GL_GEOMETRY_OUTPUT_TYPE           0x8918
#define GL_GEOMETRY_SHADER_INVOCATIONS    0x887F
#define GL_LAYER_PROVOKING_VERTEX         0x825E
#define GL_LINES_ADJACENCY                0x000A
#define GL_LINE_STRIP_ADJACENCY           0x000B
#define GL_TRIANGLES_ADJACENCY            0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY       0x000D
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS    0x8A2C
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS  0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES   0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS 0x92CF
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS   0x92D5
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS    0x90CD
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#define GL_FIRST_VERTEX_CONVENTION        0x8E4D
#define GL_LAST_VERTEX_CONVENTION         0x8E4E
#define GL_UNDEFINED_VERTEX               0x8260
#define GL_PRIMITIVES_GENERATED           0x8C87
#define GL_FRAMEBUFFER_DEFAULT_LAYERS     0x9312
#define GL_MAX_FRAMEBUFFER_LAYERS         0x9317
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_REFERENCED_BY_GEOMETRY_SHADER  0x9309
#define GL_PRIMITIVE_BOUNDING_BOX         0x92BE
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#define GL_CONTEXT_FLAGS                  0x821E
#define GL_LOSE_CONTEXT_ON_RESET          0x8252
#define GL_GUILTY_CONTEXT_RESET           0x8253
#define GL_INNOCENT_CONTEXT_RESET         0x8254
#define GL_UNKNOWN_CONTEXT_RESET          0x8255
#define GL_RESET_NOTIFICATION_STRATEGY    0x8256
#define GL_NO_RESET_NOTIFICATION          0x8261
#define GL_CONTEXT_LOST                   0x0507
#define GL_SAMPLE_SHADING                 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE       0x8C37
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#define GL_PATCHES                        0x000E
#define GL_PATCH_VERTICES                 0x8E72
#define GL_TESS_CONTROL_OUTPUT_VERTICES   0x8E75
#define GL_TESS_GEN_MODE                  0x8E76
#define GL_TESS_GEN_SPACING               0x8E77
#define GL_TESS_GEN_VERTEX_ORDER          0x8E78
#define GL_TESS_GEN_POINT_MODE            0x8E79
#define GL_ISOLINES                       0x8E7A
#define GL_QUADS                          0x0007
#define GL_FRACTIONAL_ODD                 0x8E7B
#define GL_FRACTIONAL_EVEN                0x8E7C
#define GL_MAX_PATCH_VERTICES             0x8E7D
#define GL_MAX_TESS_GEN_LEVEL             0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS 0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS 0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS 0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS      0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS 0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS 0x8E86
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS 0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS 0x8E8A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS 0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS 0x886D
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E1F
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS 0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS 0x92CE
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS 0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS 0x92D4
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS 0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS 0x90CC
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED 0x8221
#define GL_IS_PER_PATCH                   0x92E7
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#define GL_TESS_CONTROL_SHADER            0x8E88
#define GL_TESS_EVALUATION_SHADER         0x8E87
#define GL_TESS_CONTROL_SHADER_BIT        0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT     0x00000010
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_TEXTURE_BUFFER                 0x8C2A
#define GL_TEXTURE_BUFFER_BINDING         0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE        0x8C2B
#define GL_TEXTURE_BINDING_BUFFER         0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT 0x919F
#define GL_SAMPLER_BUFFER                 0x8DC2
#define GL_INT_SAMPLER_BUFFER             0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_BUFFER    0x8DD8
#define GL_IMAGE_BUFFER                   0x9051
#define GL_INT_IMAGE_BUFFER               0x905C
#define GL_UNSIGNED_INT_IMAGE_BUFFER      0x9067
#define GL_TEXTURE_BUFFER_OFFSET          0x919D
#define GL_TEXTURE_BUFFER_SIZE            0x919E
#define GL_COMPRESSED_RGBA_ASTC_4x4       0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4       0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5       0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5       0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6       0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5       0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6       0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8       0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5      0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6      0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8      0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10     0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10     0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12     0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12 0x93DD
#define GL_TEXTURE_CUBE_MAP_ARRAY         0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#define GL_SAMPLER_CUBE_MAP_ARRAY         0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW  0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY     0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F
#define GL_IMAGE_CUBE_MAP_ARRAY           0x9054
#define GL_INT_IMAGE_CUBE_MAP_ARRAY       0x905F
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY   0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D

/* GL_ES_VERSION_3_2 */
extern GL_APICALL void (* GL_APIENTRY glBlendBarrier) (void);
extern GL_APICALL void (* GL_APIENTRY glCopyImageSubData) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
extern GL_APICALL void (* GL_APIENTRY glDebugMessageControl) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
extern GL_APICALL void (* GL_APIENTRY glDebugMessageInsert) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
extern GL_APICALL void (* GL_APIENTRY glDebugMessageCallback) (GLDEBUGPROC callback, const void *userParam);
extern GL_APICALL GLuint (* GL_APIENTRY glGetDebugMessageLog) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
extern GL_APICALL void (* GL_APIENTRY glPushDebugGroup) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
extern GL_APICALL void (* GL_APIENTRY glPopDebugGroup) (void);
extern GL_APICALL void (* GL_APIENTRY glObjectLabel) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
extern GL_APICALL void (* GL_APIENTRY glGetObjectLabel) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
extern GL_APICALL void (* GL_APIENTRY glObjectPtrLabel) (const void *ptr, GLsizei length, const GLchar *label);
extern GL_APICALL void (* GL_APIENTRY glGetObjectPtrLabel) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
extern GL_APICALL void (* GL_APIENTRY glGetPointerv) (GLenum pname, void **params);
extern GL_APICALL void (* GL_APIENTRY glEnablei) (GLenum target, GLuint index);
extern GL_APICALL void (* GL_APIENTRY glDisablei) (GLenum target, GLuint index);
extern GL_APICALL void (* GL_APIENTRY glBlendEquationi) (GLuint buf, GLenum mode);
extern GL_APICALL void (* GL_APIENTRY glBlendEquationSeparatei) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
extern GL_APICALL void (* GL_APIENTRY glBlendFunci) (GLuint buf, GLenum src, GLenum dst);
extern GL_APICALL void (* GL_APIENTRY glBlendFuncSeparatei) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern GL_APICALL void (* GL_APIENTRY glColorMaski) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
extern GL_APICALL GLboolean (* GL_APIENTRY glIsEnabledi) (GLenum target, GLuint index);
extern GL_APICALL void (* GL_APIENTRY glDrawElementsBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern GL_APICALL void (* GL_APIENTRY glDrawRangeElementsBaseVertex) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern GL_APICALL void (* GL_APIENTRY glDrawElementsInstancedBaseVertex) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
extern GL_APICALL void (* GL_APIENTRY glFramebufferTexture) (GLenum target, GLenum attachment, GLuint texture, GLint level);
extern GL_APICALL void (* GL_APIENTRY glPrimitiveBoundingBox) (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
extern GL_APICALL GLenum (* GL_APIENTRY glGetGraphicsResetStatus) (void);
extern GL_APICALL void (* GL_APIENTRY glReadnPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
extern GL_APICALL void (* GL_APIENTRY glGetnUniformfv) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
extern GL_APICALL void (* GL_APIENTRY glGetnUniformiv) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
extern GL_APICALL void (* GL_APIENTRY glGetnUniformuiv) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
extern GL_APICALL void (* GL_APIENTRY glMinSampleShading) (GLfloat value);
extern GL_APICALL void (* GL_APIENTRY glPatchParameteri) (GLenum pname, GLint value);
extern GL_APICALL void (* GL_APIENTRY glTexParameterIiv) (GLenum target, GLenum pname, const GLint *params);
extern GL_APICALL void (* GL_APIENTRY glTexParameterIuiv) (GLenum target, GLenum pname, const GLuint *params);
extern GL_APICALL void (* GL_APIENTRY glGetTexParameterIiv) (GLenum target, GLenum pname, GLint *params);
extern GL_APICALL void (* GL_APIENTRY glGetTexParameterIuiv) (GLenum target, GLenum pname, GLuint *params);
extern GL_APICALL void (* GL_APIENTRY glSamplerParameterIiv) (GLuint sampler, GLenum pname, const GLint *param);
extern GL_APICALL void (* GL_APIENTRY glSamplerParameterIuiv) (GLuint sampler, GLenum pname, const GLuint *param);
extern GL_APICALL void (* GL_APIENTRY glGetSamplerParameterIiv) (GLuint sampler, GLenum pname, GLint *params);
extern GL_APICALL void (* GL_APIENTRY glGetSamplerParameterIuiv) (GLuint sampler, GLenum pname, GLuint *params);
extern GL_APICALL void (* GL_APIENTRY glTexBuffer) (GLenum target, GLenum internalformat, GLuint buffer);
extern GL_APICALL void (* GL_APIENTRY glTexBufferRange) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern GL_APICALL void (* GL_APIENTRY glTexStorage3DMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

#ifdef __cplusplus
}
#endif

#endif
