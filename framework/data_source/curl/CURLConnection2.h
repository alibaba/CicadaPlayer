//
// Created by moqi on 2020/2/12.
//

#ifndef CICADAMEDIA_CURLCONNECTION2_H
#define CICADAMEDIA_CURLCONNECTION2_H

#include "CurlMulti.h"
#include <atomic>
#include <curl/curl.h>
#include <data_source/IDataSource.h>
#include <string>
#include <utils/ringBuffer.h>

namespace Cicada {
    class CURLConnection2 {
    public:
        class CURLConnectionListener {
        public:
            virtual ~CURLConnectionListener() = default;

            virtual void onDNSResolved()
            {}
        };

    public:
        explicit CURLConnection2(Cicada::IDataSource::SourceConfig *pConfig, CurlMulti *multi, CURLConnectionListener *listener);

        void setSSLBackEnd(curl_sslbackend sslbackend);

        ~CURLConnection2();

        void reset();

        void setSource(const std::string &location, struct curl_slist *headerList);

        void setPost(bool post, int64_t size, const uint8_t *data);

        void updateSource(const std::string &location);

        void updateHeaderList(struct curl_slist *headerList);

        void setInterrupt(std::atomic_bool *inter);

        void SetResume(int64_t pos);

        int FillBuffer(uint32_t want, CurlMulti &multi);

        int short_seek(int64_t off);

        int64_t tell()
        {
            return mFilePos;
        }

        int readBuffer(void *buf, size_t size);

        const char *getResponse()
        {
            return response;
        }

        CURL *getCurlHandle()
        {
            return mHttp_handle;
        }
        void disableCallBack();

        bool isDNSResolved()
        {
            return mDNSResolved;
        }
        void pause(bool pause);

    private:
        int esayHandle_set_common_opt();

        static int sockopt_callback(void *clientp, curl_socket_t curlfd, curlsocktype purpose);

        static size_t write_response(void *ptr, size_t size, size_t nmemb, void *data);

        static size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp);

        static int xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

        static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);

        void debugHeader(bool in, char *data, size_t size);

    private:
        std::string uri;
#if 0
        char *pOverflowBuffer = nullptr;
        uint32_t overflowSize = 0;
#endif
        int sendRange = 0;
        int seekable = 0;
        int m_bFirstLoop = 0;
        std::atomic_bool *pInterrupted = nullptr;
        int so_rcv_size = 0;
        int responseSize = 0;
        std::string outHeader;
        std::string inHeader;
        struct curl_slist *reSolveList = nullptr;
        Cicada::IDataSource::SourceConfig *mPConfig = nullptr;
        CurlMulti *mMulti{nullptr};
        int64_t mFilePos = 0;
        int64_t mFileSize = -1;
        CURL *mHttp_handle = nullptr;
        RingBuffer *pRbuf = nullptr;
        int still_running = 0;
        char *response = nullptr;
        bool mDNSResolved{false};
        CURLConnectionListener *mListener{};
        bool mPaused{false};
        std::mutex mCurlCbMutex;
    };
};// namespace Cicada


#endif//CICADAMEDIA_CURLCONNECTION2_H
