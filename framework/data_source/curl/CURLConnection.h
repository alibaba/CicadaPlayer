//
// Created by moqi on 2020/2/12.
//

#ifndef CICADAMEDIA_CURLCONNECTION_H
#define CICADAMEDIA_CURLCONNECTION_H

#include <curl/curl.h>
#include <utils/ringBuffer.h>
#include <atomic>
#include <string>
#include <data_source/IDataSource.h>

namespace Cicada {
    class CURLConnection {
    public:
        explicit CURLConnection(Cicada::IDataSource::SourceConfig *pConfig);

        ~CURLConnection();

        void disconnect();

        void setSource(const std::string &location, struct curl_slist *headerList);

        void updateSource(const std::string &location);

        void setInterrupt(std::atomic_bool *inter);

        void SetResume(int64_t pos);

        void start();

        int FillBuffer(uint32_t want);

        int short_seek(int64_t off);

        int64_t tell()
        {
            return mFilePos;
        }

        int readBuffer(void *buf, size_t size);

        const char * getResponse(){
            return response;
        }

        CURL *getCurlHandle(){
            return mHttp_handle;
        }

    private:
        int esayHandle_set_common_opt();

        static int sockopt_callback(void *clientp, curl_socket_t curlfd, curlsocktype purpose);

        static size_t write_response(void *ptr, size_t size, size_t nmemb, void *data);

        static size_t write_callback(char *buffer, size_t size, size_t nitems, void *userp);

        static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);

        void debugHeader(bool in, char *data, size_t size);

    private:
        std::string uri;
        char *pOverflowBuffer = nullptr;
        uint32_t overflowSize = 0;
        int sendRange = 0;
        int seekable = 0;
        int m_bFirstLoop = 0;
        std::atomic_bool *pInterrupted = nullptr;
        int so_rcv_size = 0;
        int responseSize = 0;
        std::string outHeader = "";
        std::string inHeader = "";
        struct curl_slist *reSolveList = nullptr;
        Cicada::IDataSource::SourceConfig *mPConfig = nullptr;
        int64_t mFilePos = 0;
        int64_t mFileSize = -1;
        CURLM *multi_handle = nullptr;
        CURL *mHttp_handle = nullptr;
        RingBuffer *pRbuf = nullptr;
        int still_running = 0;
        char *response = nullptr;
    };
};


#endif //CICADAMEDIA_CURLCONNECTION_H
