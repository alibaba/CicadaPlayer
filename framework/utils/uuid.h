/*
    Single-file, STB-style, library to generate UUID:s. No dependencies
    except for OS-provided functionality.

    version 0.1, August, 2016

    Copyright (C) 2016- Fredrik Kihlander

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

    Fredrik Kihlander
*/

#ifndef UUID_H_INCLUDED
#define UUID_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct uuid { unsigned char bytes[16]; };

/**
 * Set uuid to the null_uuid.
 */
static void uuid0_generate( uuid* res );

/**
 * Generate an uuid of version 4 ( Random ) into res.
 * @note res will be the null_uuid on failure.
 */
static void uuid4_generate( uuid* res );

/**
 * Return the type of the provided uuid.
 *
 * @return 0 if it is the null-uuid
 *         1 MAC address & date-time
 *         2 DCE Security
 *         3 MD5 hash & namespace
 *         4 Random
 *         5 SHA-1 hash & namespace
 *
 *         -1 on an invalid uuid.
 */
static int uuid_type( uuid* id );

/**
 * Converts an uuid to string.
 * @param id uuid to convert.
 * @param out pointer to char-buffer where to write uuid, uuid is NOT 0-terminated
 *            and is expected to be at least 36 bytes.
 * @return out
 */
static char* uuid_to_string( uuid* id, char* out );

/**
 * Convert a string to an uuid.
 * @param str to convert.
 * @param out uuid to parse to.
 * @return true on success.
 */
static bool uuid_from_string( const char* str, uuid* out );

/**
 * Copy uuid from src to dst.
 */
static void uuid_copy( const uuid* src, uuid* dst );

#ifdef __cplusplus
}

struct _uuid_to_str
{
    char str[37];
    _uuid_to_str( uuid* id )
    {
        uuid_to_string( id, str );
        str[36] = '\0';
    }
};

/**
 * Helper macro to convert uuid to string.
 */
#define UUID_TO_STRING( id ) _uuid_to_str( id ).str

#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <string.h>

#if defined(__LINUX__) || defined(__linux__) || defined(__ANDROID__)
#  include <stdio.h>
#endif

#if defined(__ANDROID__)
#include <stdlib.h>
#endif

#if defined( _MSC_VER )
#  include <Objbase.h>
#  pragma comment(lib, "Ole32.lib")
#endif

#if defined( __APPLE__ )
#include <CoreFoundation/CFUUID.h>
#endif


#if defined(__ANDROID__)

#include <jni.h>
#include <sys/prctl.h>
#define MAX_TASK_NAME_LEN (16)

extern JavaVM *g_vm;

static jboolean attach_jvm(JNIEnv **jni_env)
{
	if ((g_vm)->GetEnv((void **)jni_env,
							  JNI_VERSION_1_4) < 0) {
        JavaVMAttachArgs lJavaVMAttachArgs;
        lJavaVMAttachArgs.version = JNI_VERSION_1_4;
        int size = MAX_TASK_NAME_LEN + 1;
        char *name = static_cast<char *>(malloc(size));
        memset(name, 0, size);
        
        if (prctl(PR_GET_NAME, (unsigned long) name, 0, 0, 0) != 0) {
            strcpy(name, "<name unknown>");
        } else {
            name[MAX_TASK_NAME_LEN] = 0;
        }

        lJavaVMAttachArgs.name = name;
        lJavaVMAttachArgs.group = nullptr;
		jint result = (g_vm)->AttachCurrentThread(jni_env, &lJavaVMAttachArgs);
        free(name);

        if (result < 0) {
			jni_env = NULL;
			return JNI_FALSE;
		}
		return JNI_TRUE;
	}

	return JNI_FALSE;
}

#define detach_jvm(attached) \
if (attached) \
   (g_vm)->DetachCurrentThread();

static int uuid_get_uuid(char* msg_buf, int buf_len)
{
	jclass uuid_class = 0;
	jmethodID get_uuid_method;
	jmethodID to_string_method;
	JNIEnv *jni_env = 0;
	jobject javaUuid = 0;
	jstring uuid_string = 0;
	int error_code = 0;
	const char *native_string;

	jboolean attached = attach_jvm(&jni_env);
	if (!jni_env) {
		error_code = 1;
		goto on_error;
	}

	uuid_class = static_cast<jclass>((jni_env)->NewGlobalRef((jni_env)->FindClass("java/util/UUID")));

	if (uuid_class == 0) {
		error_code = 2;
		goto on_error;
	}

	get_uuid_method = (jni_env)->GetStaticMethodID( uuid_class,
													"randomUUID",
													"()Ljava/util/UUID;");

	if (get_uuid_method == 0) {
		error_code = 3;
		goto on_error;
	}

	javaUuid = (jni_env)->CallStaticObjectMethod( uuid_class, get_uuid_method);
	if (javaUuid == 0) {
		error_code = 4;
		goto on_error;
	}

	to_string_method = (jni_env)->GetMethodID( uuid_class, "toString", "()Ljava/lang/String;");

	if (to_string_method == 0) {
		error_code = 5;
		goto on_error;
	}

	uuid_string = static_cast<jstring>((jni_env)->CallObjectMethod(javaUuid, to_string_method));
	if (uuid_string == 0) {
		error_code = 6;
		goto on_error;
	}

	native_string = (jni_env)->GetStringUTFChars( uuid_string, JNI_FALSE);
	if (native_string == 0) {
		error_code = 7;
		goto on_error;
	}

	strcpy(msg_buf, native_string);

	(jni_env)->ReleaseStringUTFChars( uuid_string, native_string);

	if(javaUuid != 0){
		jni_env->DeleteLocalRef(javaUuid);
	}

	if(uuid_class != 0){
		jni_env->DeleteGlobalRef(uuid_class);
	}

	detach_jvm(attached);

	return strlen(msg_buf);

	on_error:
	snprintf(msg_buf, buf_len, "-ERR%d-", error_code);

	if(uuid_string != 0){
		jni_env->DeleteLocalRef(uuid_string);
	}

	if(javaUuid != 0){
		jni_env->DeleteLocalRef(javaUuid);
	}

	if(uuid_class != 0){
		jni_env->DeleteGlobalRef(uuid_class);
	}

	detach_jvm(attached);
	return -1;
}

#endif


static char* uuid_to_string( uuid* id, char* out )
{
	static const char TOHEXCHAR[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	char* c = out;
	int src_byte = 0;
	for( int i = 0; i < 4; ++i )
	{
		*c++ = TOHEXCHAR[ ( id->bytes[src_byte] >> 4 ) & 0xF ];
		*c++ = TOHEXCHAR[ id->bytes[src_byte] & 0xF ];
		++src_byte;
	}
	*c++ = '-';

	for( int i = 0; i < 2; ++i )
	{
		*c++ = TOHEXCHAR[ ( id->bytes[src_byte] >> 4 ) & 0xF ];
		*c++ = TOHEXCHAR[ id->bytes[src_byte] & 0xF ];
		++src_byte;
	}
	*c++ = '-';

	for( int i = 0; i < 2; ++i )
	{
		*c++ = TOHEXCHAR[ ( id->bytes[src_byte] >> 4 ) & 0xF ];
		*c++ = TOHEXCHAR[ id->bytes[src_byte] & 0xF ];
		++src_byte;
	}
	*c++ = '-';

	for( int i = 0; i < 2; ++i )
	{
		*c++ = TOHEXCHAR[ ( id->bytes[src_byte] >> 4 ) & 0xF ];
		*c++ = TOHEXCHAR[ id->bytes[src_byte] & 0xF ];
		++src_byte;
	}
	*c++ = '-';

	for( int i = 0; i < 6; ++i )
	{
		*c++ = TOHEXCHAR[ ( id->bytes[src_byte] >> 4 ) & 0xF ];
		*c++ = TOHEXCHAR[ id->bytes[src_byte] & 0xF ];
		++src_byte;
	}

	return out;
}

static bool uuid_from_string( const char* str, uuid* out )
{
	char uuid_str[32];
	char* outc = uuid_str;
	for( int i = 0; i < 36; ++i )
	{
		char c = str[i];
		if( i == 8 || i == 13 || i == 18 || i == 23 )
		{
			if( c != '-' )
				return false;
		}
		else
		{
			if( !isxdigit( c ) )
				return false;
			*outc = (char)tolower( c );
			++outc;
		}
	}

#define UUID_HEXCHRTO_DEC( c ) (unsigned char)( (c) <= '9' ? ( (c) - '0' ) : 10 + (c) - ( (c) <= 'F' ? 'A' : 'a' ) )

	for( int byte = 0; byte < 16; ++byte )
	{
		unsigned char v1 = UUID_HEXCHRTO_DEC( uuid_str[ byte * 2 ] );
		unsigned char v2 = UUID_HEXCHRTO_DEC( uuid_str[ byte * 2 + 1 ] );
		out->bytes[byte] = (unsigned char)(( v1 << 4 ) | v2);
	}
#undef UUID_HEXCHRTO_DEC

	return true;
}

void uuid0_generate( uuid* res )
{
	memset( res, 0x0, sizeof(uuid) );
}

void uuid4_generate( uuid* res )
{
	uuid0_generate( res );

#if defined(__ANDROID__)
	char buf[37];
	int ret = uuid_get_uuid(buf ,37);
	if(ret <0){
	}else {
		uuid_from_string(buf, res);
	}

#elif defined(__LINUX__) || defined(__linux__)
	FILE* f = fopen( "/proc/sys/kernel/random/uuid", "rb" );
	if( f == 0x0 )
		return;

	char uuid_str[36];
	size_t read = fread( uuid_str, 1, sizeof( uuid_str ), f );
	fclose(f);
	if( read != 36 )
		return;
	uuid_from_string( uuid_str, res );
#elif defined(_MSC_VER)
	GUID g;
	HRESULT hres = CoCreateGuid( &g );
	if( hres != S_OK )
		return;
	// ... endian swap to little endian to make uuid memcpy:able ...
	g.Data1 = ( ( g.Data1 & 0x00FF ) << 24 ) | ( ( g.Data1 & 0xFF00 ) << 8) | ( ( g.Data1 >> 8 ) & 0xFF00 ) | ( ( g.Data1 >> 24 ) & 0x00FF );
	g.Data2 = (WORD)( ( ( g.Data2 & 0x00FF ) << 8 ) | ( ( g.Data2 & 0xFF00 ) >> 8 ) );
	g.Data3 = (WORD)( ( ( g.Data3 & 0x00FF ) << 8 ) | ( ( g.Data3 & 0xFF00 ) >> 8 ) );
	memcpy( res->bytes, &g, sizeof( res->bytes ) );
#elif defined( __APPLE__ )
	CFUUIDRef new_uuid = CFUUIDCreate(0x0);
	CFUUIDBytes bytes = CFUUIDGetUUIDBytes( new_uuid );

	res->bytes[0] = bytes.byte0;
	res->bytes[1] = bytes.byte1;
	res->bytes[2] = bytes.byte2;
	res->bytes[3] = bytes.byte3;
	res->bytes[4] = bytes.byte4;
	res->bytes[5] = bytes.byte5;
	res->bytes[6] = bytes.byte6;
	res->bytes[7] = bytes.byte7;
	res->bytes[8] = bytes.byte8;
	res->bytes[9] = bytes.byte9;
	res->bytes[10] = bytes.byte10;
	res->bytes[11] = bytes.byte11;
	res->bytes[12] = bytes.byte12;
	res->bytes[13] = bytes.byte13;
	res->bytes[14] = bytes.byte14;
	res->bytes[15] = bytes.byte15;
	CFRelease ( new_uuid );
#else
#  error "unhandled platform"
#endif
}

int uuid_type( uuid* id )
{
	switch( ( id->bytes[6] & 0xF0 ) >> 4 )
	{
		case 0:
			for( int i = 0; i < 16; ++i )
				if( id->bytes[i] != 0 )
					return -1;
			return 0;
		case 1:
			return 1;
		case 2:
			return 2;
		case 3:
			switch( ( id->bytes[8] & 0xF0 ) >> 4 )
			{
				case 8:
				case 9:
				case 10:
				case 11:
					return 4;
				default:
					return -1;
			}
			break;
		case 4:
			switch( ( id->bytes[8] & 0xF0 ) >> 4 )
			{
				case 8:
				case 9:
				case 10:
				case 11:
					return 4;
				default:
					return -1;
			}
			break;
		case 5:
			return 5;
	}
	return -1;
}

void uuid_copy( const uuid* src, uuid* dst )
{
	memcpy( dst, src, sizeof(uuid) );
}

#ifdef __cplusplus
}
#endif

#endif // UUID_H_INCLUDED