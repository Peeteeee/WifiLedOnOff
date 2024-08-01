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
static const char *TAG2 = "LOAD";
static const char *TAG3 = "ESP32_JSON";

static uint8_t led_state = 0;
#define relePin GPIO_NUM_2

// WiFi SSID a heslo
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"
#define MAX_DATA_LENGTH 256
// Struktura pro uložení textu
typedef struct {
    char text[MAX_DATA_LENGTH];
} Data;

#endif