#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include "curl.h"

size_t WriteCallback(void *contents, size_t size, size_t nmemb, struct StringBuffer *buffer)
{
    size_t realsize = size * nmemb;

    char *ptr = realloc(buffer->data, buffer->size + realsize + 1);
    if (ptr == NULL)
    {
        printf("Erreur de réallocation de mémoire.\n");
        return 0;
    }

    buffer->data = ptr;
    memcpy(&(buffer->data[buffer->size]), contents, realsize);
    buffer->size += realsize;
    buffer->data[buffer->size] = '\0';

    return realsize;
}

void call(const char *url, struct StringBuffer *buffer)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    buffer->data = NULL;
    buffer->size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            printf("Erreur lors de l'exécution de la requête curl : %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }
    else
        printf("Erreur lors de l'initialisation de curl.\n");
    curl_global_cleanup();
}

// int main()
// {
//     const char *url = "https://pokeapi.co/api/v2/pokemon/ditto";

//     struct StringBuffer buffer;
//     call(url, &buffer);

//     printf("Réponse de la requête :\n%s\n", buffer.data);

//     free(buffer.data);

//     return 0;
// }