#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "hal/DiscordAlert.h"

static const char* webhook_URL = "https://discord.com/api/webhooks/1438726790259540070/fZlHT6fpXPuG-DSau6iggXdJoXXSUf1pFYwnEZrkkCh25yyAGM5BCRynv1HC05g16n5f";
void discordInit(void){
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void discordCleanup(void){
    curl_global_cleanup();
}

void send_discord_alert(const char *webhook_url, const char *msg)
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init() failed\n");
        return;
    }

    // JSON payload: { "content": "your message" }
    char json[512];
    snprintf(json, sizeof(json), "{\"content\":\"%s\"}", msg);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, webhook_url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Discord webhook failed: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}