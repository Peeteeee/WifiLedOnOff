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


#define FILE_PATH "/spiffs/index.html" // Cesta k HTML souboru
#define FILE_PATH2 "/spiffs/druh_postriku.txt"

static const char *TAG = "HTTP_SERVER";
static const char *TAG2 = "LOAD";

static uint8_t led_state = 0;
#define relePin GPIO_NUM_2

// WiFi SSID a heslo
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"



#endif