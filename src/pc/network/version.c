#include <stdio.h>
#include <curl/curl.h>
#include "version.h"
#include "types.h"
#include "pc/debuglog.h"
#include "pc/djui/djui.h"

static char sVersionString[MAX_VERSION_LENGTH] = { 0 };
#define VERSION_TEXT "beta "

struct string {
    char *ptr;
    size_t len;
};

char* get_version(void) {
    snprintf(sVersionString, MAX_VERSION_LENGTH, "%s%d.%d", VERSION_TEXT, VERSION_NUMBER, MINOR_VERSION_NUMBER);
    return sVersionString;
}

static size_t write(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

void check_for_updates(void) {
    CURL *curl = curl_easy_init();
    CURLcode response;

    curl_global_init(CURL_GLOBAL_ALL);
    int substr_length = strlen("#define VERSION_NUMBER ");

    if (curl) {
        struct string s;
        s.len = 0;
        s.ptr = malloc(s.len + 1);
        if (s.ptr == NULL) {
            fprintf(stderr, "malloc() failed\n");
        }
        s.ptr[0] = '\0';

        curl_easy_setopt(
            curl, CURLOPT_URL,
            "https://raw.githubusercontent.com/djoslin0/sm64ex-coop/coop/src/pc/network/version.h");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        response = curl_easy_perform(curl);

        if (response != CURLE_OK) {
            LOG_ERROR("failed to check updates: %s\n", curl_easy_strerror(response));
        } else {
            char string[800];
            snprintf(string, 800, "%s", s.ptr);
            char *token = strtok(string, "\n");

            while (token != NULL) {
                if (strncmp("#define VERSION_NUMBER ", token, substr_length) == 0) {
                    int string_length = strlen(token);

                    for (int i = substr_length; i < string_length; i++) {
                        for (int j = 0; j < string_length; j++) {
                            if (i > substr_length) {
                                token[j] = token[j + substr_length];
                            }
                        }
                    }
                    int latestVersionNumber;
                    latestVersionNumber = atoi(token);
                    LOG_INFO("latest version number is %d", latestVersionNumber);
                    if (latestVersionNumber > VERSION_NUMBER) {
                        char message[256] = { 0 };
                        snprintf(message, 256, "\\#fff982\\Update available!\\#dcdcdc\\ Your version: %d Latest version: %d",
                                 VERSION_NUMBER, latestVersionNumber);
                        djui_popup_create(message, 3);
                    }
                    break;
                }
                token = strtok(NULL, "\n");
            }
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}
