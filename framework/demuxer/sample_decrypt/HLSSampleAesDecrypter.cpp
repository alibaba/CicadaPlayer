//
// Created by moqi on 2018/10/23.
//

#define LOG_TAG "HLSSampleAesDecrypter"

#include "HLSSampleAesDecrypter.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#ifndef WIN32

    #include <unistd.h>

#endif

#include <cassert>
#include <utils/frame_work_log.h>
#include <cstring>
#include "../decrypto/avAESDecrypt.h"

using namespace Cicada;
using namespace std;

HLSSampleAesDecrypter::HLSSampleAesDecrypter()
{
    mDecrypt = std::unique_ptr<Cicada::IAESDecrypt>(new avAESDecrypt());
}

int HLSSampleAesDecrypter::SetOption(const char *key, uint8_t *buffer, int size)
{
    std::string skey = key;

    if (skey == "decryption key" || skey == "decryption IV") {
        if (size != IAESDecrypt::BLOCK_SIZE) {
            return -1;
        }

        if (skey == "decryption key") {
            mValidKeyInfo = (mDecrypt->setKey(buffer, 8 * IAESDecrypt::BLOCK_SIZE) == 0);
        } else {
            memcpy(mIvec, buffer, IAESDecrypt::BLOCK_SIZE);
        }
    } else if (skey == "decryption KEYFORMAT") {
        // TODO:
    }

    return 0;
}

int HLSSampleAesDecrypter::decrypt(AFCodecID codecId, uint8_t *buffer, int size)
{
    if (!mValidKeyInfo) {
        AF_LOGE("key info is not valid\n");
        return size;
    }

    int ret = size;

    switch (codecId) {
        case AF_CODEC_ID_H264:
            ret = decryptH264Video(buffer, size);
            break;

        case AF_CODEC_ID_AAC:
            ret = decryptAACAudio(buffer, size);
            break;

        default:
            break;
    }

    return ret;
}


HLSSampleAesDecrypter::~HLSSampleAesDecrypter()
{
}

static uint8_t *find_naulunit(uint8_t *buffer, int size, int *nal_size, int &startcode_len)
{
    int index = 0;
    *nal_size = 0;
    int find_start_pos = -1;
    int find_end_pos = -1;

    while (index < size) {
        if (index + 3 < size && buffer[index] == 0x00 && buffer[index + 1] == 0x00 && buffer[index + 2] == 0x00 &&
                buffer[index + 3] == 0x01) {
            if (find_start_pos == -1) {
                index += 4;
                find_start_pos = index;
                startcode_len = 4;
                continue;
            } else {
                find_end_pos = index;
                *nal_size = find_end_pos - find_start_pos;
                return buffer + find_start_pos;
            }
        }

        if (index + 2 < size && buffer[index] == 0x00 && buffer[index + 1] == 0x00 && buffer[index + 2] == 0x01) {
            if (find_start_pos == -1) {
                index += 3;
                find_start_pos = index;
                startcode_len = 3;
                continue;
            } else {
                find_end_pos = index;
                *nal_size = find_end_pos - find_start_pos;
                return buffer + find_start_pos;
            }
        }

        index++;
    }

    if (find_start_pos == -1) {
        assert(0);
        *nal_size = 0;
        return nullptr;
    }

    if (find_end_pos == -1) {
        *nal_size = size - find_start_pos;
        return buffer + find_start_pos;
    }

    return nullptr;
}

static int remove_nalunit_prevention(uint8_t *nal_unit, int nal_size, uint8_t *dst, int &dst_pos)
{
    uint8_t *tmp = nal_unit;
    uint8_t *tmp_end = nal_unit + nal_size;
    int size = dst_pos;
    //skip naltype
    dst[dst_pos++] = *tmp;
    tmp++;

    while (tmp < tmp_end) {
        if (tmp + 3 < tmp_end && tmp[0] == 0x00 && tmp[1] == 0x00 && tmp[2] == 0x03 &&
                (tmp[3] == 0x00 || tmp[3] == 0x01 || tmp[3] == 0x02 || tmp[3] == 0x03)) {
            dst[dst_pos++] = tmp[0];
            dst[dst_pos++] = tmp[1];
            dst[dst_pos++] = tmp[3];
            nal_size--;
            tmp += 4;
            continue;
        }

        dst[dst_pos++] = *tmp;
        tmp++;
    }

    assert(nal_size == (dst_pos - size));
    return nal_size;
}

void HLSSampleAesDecrypter::decrypt_nalunit(uint8_t *nal_unit, int nal_size)
{
    uint8_t packet_iv_tmp[IAESDecrypt::BLOCK_SIZE];
    memcpy(packet_iv_tmp, mIvec, IAESDecrypt::BLOCK_SIZE);
    /*
     Encrypted_nal_unit () {

        nal_unit_type_byte                // 1 byte
        unencrypted_leader                // 31 bytes

        while (bytes_remaining() > 0) {
            if (bytes_remaining() > 16) {
                encrypted_block           // 16 bytes
            }
            unencrypted_block           // MIN(144, bytes_remaining()) bytes
        }
     }
     */
    nal_unit += VIDEO_CLEAR_LEAD;
    nal_size -= VIDEO_CLEAR_LEAD;

    while (nal_size > IAESDecrypt::BLOCK_SIZE) {
        if (nal_size > IAESDecrypt::BLOCK_SIZE) {
            mDecrypt->decrypt(nal_unit, nal_unit, 1, packet_iv_tmp);
            nal_size -= IAESDecrypt::BLOCK_SIZE;
            nal_unit += IAESDecrypt::BLOCK_SIZE;
        }

        // unencrypted_block
        int sz = std::min(nal_size, (9 * IAESDecrypt::BLOCK_SIZE));
        nal_size -= sz;
        nal_unit += sz;
    }
}

int HLSSampleAesDecrypter::decryptH264Video(uint8_t *buffer, int size)
{
    uint8_t *h264_frame_end = buffer + size;
    uint8_t *tmp = buffer;
    uint8_t *dst = (uint8_t *) malloc(size);
    int dst_pos = 0;

    while (tmp < h264_frame_end) {
        //find nal unit, not including startcode
        int nal_size = 0;
        int startcode_len = 4;
        uint8_t *nal_unit = find_naulunit(tmp, (int) (h264_frame_end - tmp), &nal_size, startcode_len);

        if (nal_unit == nullptr || nal_size == 0) {
            break;
        }

        if (startcode_len == 4) {
            dst[dst_pos++] = 0x00;
            dst[dst_pos++] = 0x00;
            dst[dst_pos++] = 0x00;
            dst[dst_pos++] = 0x01;
        } else if (startcode_len == 3) {
            dst[dst_pos++] = 0x00;
            dst[dst_pos++] = 0x00;
            dst[dst_pos++] = 0x01;
        } else {
            assert(0);
            return size;
        }

        int nal_type = nal_unit[0] & 0x1F;

        if ((nal_type != 1 && nal_type != 5) ||
                (nal_size <= VIDEO_CLEAR_LEAD + IAESDecrypt::BLOCK_SIZE)) {
            memcpy(dst + dst_pos, nal_unit, static_cast<size_t>(nal_size));
            dst_pos += nal_size;
        } else {
            uint8_t *new_nal_start = dst + dst_pos;
            //remove prevension
            int new_nal_size = remove_nalunit_prevention(nal_unit, nal_size, dst, dst_pos);
            //decrypt
            decrypt_nalunit(new_nal_start, new_nal_size);
        }

        tmp = nal_unit + nal_size;
    }

    memcpy(buffer, dst, dst_pos);
    free(dst);
    return dst_pos;
}

int HLSSampleAesDecrypter::decryptAACAudio(uint8_t *buffer, int size)
{
    uint8_t packet_iv[IAESDecrypt::BLOCK_SIZE];
    memcpy(packet_iv, mIvec, IAESDecrypt::BLOCK_SIZE);
    int aac_frame_size = ((buffer[3] & 0x03) << 11) + (buffer[4] << 3) + ((buffer[5] & 0xE0) >> 5);

    if (aac_frame_size > size) {
        aac_frame_size = size;
    }

    //Encrypted_AAC_Frame () {
    //    ADTS_Header                        // 7 or 9 bytes
    //    unencrypted_leader                 // 16 bytes
    //    while (bytes_remaining() >= 16) {
    //        encrypted_block                // 16 bytes
    //    }
    //    unencrypted_trailer                // 0-15 bytes
    //}
    int offset = (buffer[1] & 0x01) ? 7 : 9;
    offset += AUDIO_CLEAR_LEAD;
    int remainingBytes = aac_frame_size - offset;

    if (remainingBytes >= IAESDecrypt::BLOCK_SIZE) {
        int encrypt_size = (remainingBytes / IAESDecrypt::BLOCK_SIZE);
        mDecrypt->decrypt(buffer + offset, buffer + offset, encrypt_size, packet_iv);
    }

    return size;
}
