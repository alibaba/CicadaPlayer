//
// Created by moqi on 2019-04-12.
//
#include <curl/multi.h>

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    (void)contents;
    (void)userp;
    return realsize;
}

int main()
{
    CURLM *m = NULL;
    CURLMsg *msg; /* for picking up messages with the transfer status */
    int msgs_left; /* how many messages are left */
    CURL *handle = nullptr;
    curl_global_init(CURL_GLOBAL_ALL);
    m = curl_multi_init();
    handle = curl_easy_init();
    int running = 0;
    int handlenum = 0;
    curl_easy_setopt(handle, CURLOPT_URL, "https://www.baidu.com");
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);
    curl_multi_add_handle(m, handle);

    for (;;) {
        curl_multi_perform(m, &running);

        /* See how the transfers went */
        do {
            msg = curl_multi_info_read(m, &msgs_left);

            if (msg && msg->msg == CURLMSG_DONE) {
//                printf("Handle %d Completed with status %d\n", i, msg->data.result);
                curl_multi_remove_handle(m, handle);
                break;
            }
        } while (msg);

        if (!running) {
            break; /* done */
        }
    }

    curl_easy_setopt(handle, CURLOPT_URL, "https://www.baidu.com/img/bd_logo1.png");
    curl_multi_add_handle(m, handle);
    printf("xxxxxxxxxxxxxxxxx\n\n\n");

    for (;;) {
        curl_multi_perform(m, &running);

        /* See how the transfers went */
        do {
            msg = curl_multi_info_read(m, &msgs_left);

            if (msg && msg->msg == CURLMSG_DONE) {
//                printf("Handle %d Completed with status %d\n", i, msg->data.result);
                curl_multi_remove_handle(m, handle);
                break;
            }
        } while (msg);

        if (!running) {
            break; /* done */
        }
    }
}

