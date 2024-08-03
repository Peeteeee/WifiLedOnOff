#ifndef __HX711_H__
#define __HX711_H__

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "driver/gpio.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "cJSON.h"

#define FILE_PATH "/spiffs/index.html"
#define FILE_PATH2 "/spiffs/druh_postriku.txt"

#define IP_ADRESA_ESP32 "192.168.0.111"

static const char *TAG = "HTTP_SERVER";
static const char *TAG4 = "JSON_Parser";
// WiFi SSID a heslo
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"
#define MAX_DATA_LENGTH 256

int pocetPostriku = 0;
int next_id = 1; // Globální proměnná pro sledování dalšího ID

static const char *TAG = "HTTP_SERVER";
static const char *TAG4 = "JSON_Parser";
static const char *mujTag = "MUJTAG";
static const char *TAG5 = "TIME";

bool zacinameMichat = false;
bool zacinamePoustetVodu = false;
bool zacinameVazit = false;
bool nezahajenoPousteniVodyTlacitkem = true;
typedef struct
{
    int id;
    char nazev_pripravku[50];
    char osetrovana_plodina[50];
    double mnozstvi_postriku;
    double pomer_michani;
    char doba_postriku[11]; // Formát: YYYY-MM-DD (10 znaků + 1 pro nulový terminátor)
} PostrikData;

#endif