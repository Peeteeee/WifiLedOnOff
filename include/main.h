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
#include "esp_sntp.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include <sys/time.h>
#include <time.h>
#include "esp_system.h"
#include <lcd1602.h>

#define RS GPIO_NUM_2
#define EN GPIO_NUM_4
#define D4 GPIO_NUM_5
#define D5 GPIO_NUM_21
#define D6 GPIO_NUM_22
#define D7 GPIO_NUM_23

#define FILE_PATH "/spiffs/uvodniStrana.html"
#define FILE_PATH2 "/spiffs/vkladaniDat.html"
#define FILE_PATH3 "/spiffs/poleStruktur.bin"
#define IP_ADRESA_ESP32 "192.168.0.111"
#define relePin GPIO_NUM_2
#define MAX_RUZNYCH_POSTRIKU 20
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"
#define MAX_DATA_LENGTH 256
#define CHUNK_SIZE 1024
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
    char osetrovana_plodina[81];
    double mnozstvi_postriku;
    double pomer_michani;
    char doba_postriku[11];
} PostrikData;

#endif