#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "./platform/platform_config.h"


#ifndef DIRECTOR_MAC_USE_DISPLAY_LINK_THREAD
#define DIRECTOR_MAC_USE_DISPLAY_LINK_THREAD 1
#endif

#ifndef NODE_RENDER_SUBPIXEL
#define NODE_RENDER_SUBPIXEL 1
#endif


#ifndef SPRITEBATCHNODE_RENDER_SUBPIXEL
#define SPRITEBATCHNODE_RENDER_SUBPIXEL    1
#endif


#ifndef TEXTURE_ATLAS_USE_VAO
#define TEXTURE_ATLAS_USE_VAO 1
#endif


#ifndef SPRITE_DEBUG_DRAW
#define SPRITE_DEBUG_DRAW 0
#endif


/** Use 3D navigation API */
#ifndef USE_NAVMESH
#define USE_NAVMESH 1
#endif

/** Use culling or not. */
#ifndef USE_CULLING
#define USE_CULLING 1
#endif

#ifndef USE_PNG
#define USE_PNG  1
#endif // USE_PNG


#ifndef USE_JPEG
#define USE_JPEG  0
#endif // USE_JPEG

#ifndef USE_WEBP
#if (TARGET_PLATFORM != PLATFORM_WINRT)
#define USE_WEBP  0
#endif
#endif // USE_WEBP

#endif // __CONFIG_H__
