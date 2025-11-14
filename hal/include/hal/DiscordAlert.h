#ifndef DISCORDALERT_H
#define DISCORDALERT_H

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>


void discordInit(void);
void discordCleanup(void);
void sendDiscordAlert(const char *webhookURL, const char *msg);

#endif
