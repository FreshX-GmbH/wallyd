#include "connect.h"

bool longpoll_active = false;
void *longpoll_curl;
char *longpoll_url = NULL;
mybuf_t *lpbuf = NULL;

size_t HeaderCallback( void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t bytes = size * nmemb;
    if (!longpoll_active && strlen(ptr) >= 0x10)// && strstr(ptr,"X-Long-Polling:"))
    {
        //Curl::longpoll_url = hdr.substr(0x10);
        //Curl::longpoll_url = Curl::longpoll_url.substr(0, Curl::longpoll_url.length()-2);
        slog(LVL_NOISY,DEBUG, "Longpoll url %s -> %s", longpoll_url, ptr);
        longpoll_active = true;
    }
    return bytes;
}

static size_t ResponseCallback(void *buffer, size_t size, size_t nmemb, void *userp)
{
    mybuf_t *buf = (mybuf_t *) userp;
    size_t total = size * nmemb;

    if (buf->limit - buf->len < total)
    {
        buf = mybuf_size(buf, buf->limit + total);
        slog(LVL_NOISY,DEBUG,"%s",buf);
    }

    mybuf_concat(buf, buffer, total);

    return total;
}

char *url_call(char *url)
{
    slog(LVL_NOISY,DEBUG,"Reading data from %s",url);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    mybuf_t *buf = mybuf_size(NULL, BUFFER_SIZE);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &ResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "WallyTV (https://github.com/FreshXOpenSource/wally, support@freshx.de)");

    struct curl_slist *hs = curl_slist_append(NULL, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK){
        slog(LVL_QUIET,ERROR,"curl_easy_perform failed: %s", curl_easy_strerror(res));
        return NULL;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(hs);

    char *js = mybuf_tostr(buf);
    free(buf->data);
    free(buf);

    slog(LVL_NOISY,DEBUG,"Read : %s",js);
    return js;
}

void *url_init(void){
    curl_global_init(CURL_GLOBAL_ALL);
    longpoll_curl = curl_easy_init();
    return longpoll_curl;
}

char *url_longpoll(char *url, int timeout, int type, char *postdata)
{
    longpoll_url = url;
    lpbuf = mybuf_size(NULL, BUFFER_SIZE);
    if(type == LONGPOLL_INIT){
        slog(LVL_NOISY,DEBUG,"Setting up long polling to %s",url);
    }
    struct curl_slist *headerlist = curl_slist_append(NULL, "Accept: application/json");
    headerlist = curl_slist_append(headerlist, "Content-Type: application/json");
    headerlist = curl_slist_append(headerlist, "User-Agent: WallyTV (https://github.com/FreshXOpenSource/wally, support@freshx.de)");

    curl_easy_setopt(longpoll_curl, CURLOPT_URL, url);

    curl_easy_setopt(longpoll_curl, CURLOPT_WRITEFUNCTION, &ResponseCallback);
    curl_easy_setopt(longpoll_curl, CURLOPT_WRITEDATA, lpbuf);

    curl_easy_setopt(longpoll_curl, CURLOPT_HEADERFUNCTION, &HeaderCallback);
    curl_easy_setopt(longpoll_curl, CURLOPT_HTTPHEADER, headerlist);

    curl_easy_setopt(longpoll_curl, CURLOPT_CONNECTTIMEOUT, timeout);
    if(type == LONGPOLL_GET){
        curl_easy_setopt(longpoll_curl, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(longpoll_curl, CURLOPT_POST, 0);
    }
    if(type == LONGPOLL_POST){
        curl_easy_setopt(longpoll_curl, CURLOPT_POST, 1);
        curl_easy_setopt(longpoll_curl, CURLOPT_COPYPOSTFIELDS, postdata);
    }
    CURLcode code = curl_easy_perform(longpoll_curl);
    if(code != CURLE_OK)
    {
        switch(code){
            case CURLE_COULDNT_CONNECT:
                slog(LVL_QUIET,ERROR,"Could not connect to server");
                break;
            case CURLE_GOT_NOTHING:
                slog(LVL_QUIET,ERROR,"Longpolling timed out.");
                break;
            default:
                slog(LVL_QUIET,ERROR,"Error getting command (%d). See http://curl.haxx.se/libcurl/c/libcurl-errors.html", code);
                break;
        }
    }

    char *js = mybuf_tostr(lpbuf);
    mybuf_destroy(lpbuf);
    return js;
}

void url_destroy_longpoll(void *longpoll_curl){
    curl_easy_cleanup(longpoll_curl);
    curl_global_cleanup();
    return;
}

