find_library(VIDEO_TOOL_BOX VideoToolbox)
find_library(AUDIO_TOOL_BOX AudioToolbox)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COREFOUNDATION CoreFoundation)
#find_library(VIDEODECODERACCELERATION VideoDecodeAcceleration)
find_library(COREFOUNDATION CoreFoundation)
find_library(SECURITY Security)

set(FRAMEWORK_LIBS
        #${COCOA}
        ${VIDEO_TOOL_BOX}
        ${AUDIO_TOOL_BOX}
        ${COREMEDIA}
        ${COREVIDEO}
        #        ${VIDEODECODERACCELERATION}
        ${COREFOUNDATION}
        ${SECURITY}
        ${COREFOUNDATION})