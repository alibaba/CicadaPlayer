/* ************************************************************************
 *       Filename:  utf.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2016年10月21日 09时50分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "utf.h"

static boolean isLegalUTF8(const UTF8 *source, int length)
{
    UTF8 a;
    const UTF8 *srcptr = NULL;

    if (NULL == source) {
        printf("ERR, isLegalUTF8: source=%p\n", source);
        return FALSE;
    }
    srcptr = source + length;

    switch (length) {
        default:
            printf("ERR, isLegalUTF8 1: length=%d\n", length);
            return FALSE;
            /* Everything else falls through when "TRUE"... */
        case 4:
            if ((a = (*--srcptr)) < 0x80 || a > 0xBF) {
                printf("ERR, isLegalUTF8 2: length=%d, a=%x\n", length, a);
                return FALSE;
            }
        case 3:
            if ((a = (*--srcptr)) < 0x80 || a > 0xBF) {
                printf("ERR, isLegalUTF8 3: length=%d, a=%x\n", length, a);
                return FALSE;
            }
        case 2:
            if ((a = (*--srcptr)) > 0xBF) {
                printf("ERR, isLegalUTF8 4: length=%d, a=%x\n", length, a);
                return FALSE;
            }
            switch (*source) {
                /* no fall-through in this inner switch */
                case 0xE0:
                    if (a < 0xA0) {
                        printf("ERR, isLegalUTF8 1: source=%x, a=%x\n", *source, a);
                        return FALSE;
                    }
                    break;
                case 0xED:
                    if (a > 0x9F) {
                        printf("ERR, isLegalUTF8 2: source=%x, a=%x\n", *source, a);
                        return FALSE;
                    }
                    break;
                case 0xF0:
                    if (a < 0x90) {
                        printf("ERR, isLegalUTF8 3: source=%x, a=%x\n", *source, a);
                        return FALSE;
                    }
                    break;
                case 0xF4:
                    if (a > 0x8F) {
                        printf("ERR, isLegalUTF8 4: source=%x, a=%x\n", *source, a);
                        return FALSE;
                    }
                    break;
                default:
                    if (a < 0x80) {
                        printf("ERR, isLegalUTF8 5: source=%x, a=%x\n", *source, a);
                        return FALSE;
                    }
            }
        case 1:
            if (*source >= 0x80 && *source < 0xC2) {
                printf("ERR, isLegalUTF8: source=%x\n", *source);
                return FALSE;
            }
    }
    if (*source > 0xF4)
        return FALSE;
    return TRUE;
}

ConversionResult Utf8_To_Utf16(const UTF8 *sourceStart, UTF16 *targetStart, size_t outLen, ConversionFlags flags)
{
    ConversionResult result = conversionOK;
    const UTF8 *source = sourceStart;
    UTF16 *target = targetStart;
    UTF16 *targetEnd = targetStart + outLen / 2;
    const UTF8 *sourceEnd = NULL;

    if ((NULL == source) || (NULL == targetStart)) {
        printf("ERR, Utf8_To_Utf16: source=%p, targetStart=%p\n", source, targetStart);
        return conversionFailed;
    }
    sourceEnd = strlen((const char *) sourceStart) + sourceStart;

    while (*source) {
        UTF32 ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        if (source + extraBytesToRead >= sourceEnd) {
            printf("ERR, Utf8_To_Utf16----sourceExhausted: source=%p, extraBytesToRead=%d, sourceEnd=%p\n", source,
                   extraBytesToRead, sourceEnd);
            result = sourceExhausted;
            break;
        }
        /* Do this check whether lenient or strict */
        if (!isLegalUTF8(source, extraBytesToRead + 1)) {
            printf("ERR, Utf8_To_Utf16----isLegalUTF8 return FALSE: source=%p, extraBytesToRead=%d\n", source,
                   extraBytesToRead);
            result = sourceIllegal;
            break;
        }
        /*
        * The cases all fall through. See "Note A" below.
        */
        switch (extraBytesToRead) {
            case 5:
                ch += *source++;
                ch <<= 6; /* remember, illegal UTF-8 */
            case 4:
                ch += *source++;
                ch <<= 6; /* remember, illegal UTF-8 */
            case 3:
                ch += *source++;
                ch <<= 6;
            case 2:
                ch += *source++;
                ch <<= 6;
            case 1:
                ch += *source++;
                ch <<= 6;
            case 0:
                ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        if (target >= targetEnd) {
            source -= (extraBytesToRead + 1); /* Back up source pointer! */
            printf("ERR, Utf8_To_Utf16----target >= targetEnd: source=%p, extraBytesToRead=%d\n", source,
                   extraBytesToRead);
            result = targetExhausted;
            break;
        }
        if (ch <= UNI_MAX_BMP) {
            /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
                if (flags == strictConversion) {
                    source -= (extraBytesToRead + 1); /* return to the illegal value itself */
                    printf("ERR, Utf8_To_Utf16----ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END: source=%p, extraBytesToRead=%d\n",
                           source, extraBytesToRead);
                    result = sourceIllegal;
                    break;
                } else {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } else {
                *target++ = (UTF16) ch; /* normal case */
            }
        } else if (ch > UNI_MAX_UTF16) {
            if (flags == strictConversion) {
                result = sourceIllegal;
                source -= (extraBytesToRead + 1); /* return to the start */
                printf("ERR, Utf8_To_Utf16----ch > UNI_MAX_UTF16: source=%p, extraBytesToRead=%d\n", source,
                       extraBytesToRead);
                break; /* Bail out; shouldn't continue */
            } else {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            if (target + 1 >= targetEnd) {
                source -= (extraBytesToRead + 1); /* Back up source pointer! */
                printf("ERR, Utf8_To_Utf16----target + 1 >= targetEnd: source=%p, extraBytesToRead=%d\n", source,
                       extraBytesToRead);
                result = targetExhausted;
                break;
            }
            ch -= halfBase;
            *target++ = (UTF16) ((ch >> halfShift) + UNI_SUR_HIGH_START);
            *target++ = (UTF16) ((ch & halfMask) + UNI_SUR_LOW_START);
        }
    }
    return result;
}

int Utf16_To_Utf8(const UTF16 *sourceStart, UTF8 *targetStart, size_t outLen, ConversionFlags flags)
{
    int result = 0;
    const UTF16 *source = sourceStart;
    UTF8 *target = targetStart;
    UTF8 *targetEnd = targetStart + outLen;

    if ((NULL == source) || (NULL == targetStart)) {
        printf("ERR, Utf16_To_Utf8: source=%p, targetStart=%p\n", source, targetStart);
        return conversionFailed;
    }

    while (*source) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        const UTF32 byteMask = 0xBF;
        const UTF32 byteMark = 0x80;
        const UTF16 *oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if (*source) {
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                } else if (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                break;
            }
        } else if (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                break;
            }
        }
        /* Figure out how many bytes the result will require */
        if (ch < (UTF32) 0x80) {
            bytesToWrite = 1;
        } else if (ch < (UTF32) 0x800) {
            bytesToWrite = 2;
        } else if (ch < (UTF32) 0x10000) {
            bytesToWrite = 3;
        } else if (ch < (UTF32) 0x110000) {
            bytesToWrite = 4;
        } else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        if (target > targetEnd) {
            source = oldSource; /* Back up source pointer! */
            target -= bytesToWrite;
            result = targetExhausted;
            break;
        }
        switch (bytesToWrite) { /* note: everything falls through. */
            case 4:
                *--target = (UTF8) ((ch | byteMark) & byteMask);
                ch >>= 6;
            case 3:
                *--target = (UTF8) ((ch | byteMark) & byteMask);
                ch >>= 6;
            case 2:
                *--target = (UTF8) ((ch | byteMark) & byteMask);
                ch >>= 6;
            case 1:
                *--target = (UTF8) (ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    return result;
}

#if 0
int main(int argc, char *argv[])
{
    int i=0;
    UTF8 buf8[256]="";
    UTF16 buf16[256]={0};
    strcpy(buf8,"程序员");
    Utf8_To_Utf16(buf8,buf16,sizeof(buf16),strictConversion);
    printf("\nUTF-8 => UTF-16 = ");
    while(buf16[i])
    {
        printf("%#x  ",buf16[i]);
        i++;
    }

    memset(buf8,0,sizeof(buf8));
    memset(buf16,0,sizeof(buf16));
    buf16[0]=0x7a0b;
    buf16[1]=0x5e8f;
    buf16[2]=0x5458;
    Utf16_To_Utf8 (buf16, buf8, sizeof(buf8) , strictConversion);
    printf("\nUTF-16 => UTF-8 = %s\n\n",buf8);
    return 0;
}
#endif

