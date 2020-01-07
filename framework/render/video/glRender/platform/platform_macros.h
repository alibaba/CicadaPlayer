#ifndef __PLATFORM_MACROS_H__
#define __PLATFORM_MACROS_H__

#define SAFE_DELETE(p)           do { if(p) { delete (p); (p) = nullptr; } } while(0)
#define SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)
#define SAFE_FREE(p)             do { if(p) { free(p); (p) = nullptr; } } while(0)
#define BREAK_IF(cond)           if(cond) break

#define SAFE_RETAIN(p)           do { if(p) { (p)->Retain(); } } while(0)
#define SAFE_RELEASE(p)          do { if(p) { (p)->Release(); } } while(0)

#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#endif // __PLATFORM_MACROS_H__
