#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
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
#include <math.h>
#include "freertos/semphr.h"
#include <hx711.h>
#include <inttypes.h>
#define RS GPIO_NUM_2
#define EN GPIO_NUM_4
#define D4 GPIO_NUM_5
#define D5 GPIO_NUM_21
#define D6 GPIO_NUM_22
#define D7 GPIO_NUM_23

#define tlacitko1 GPIO_NUM_12
#define tlacitko2 GPIO_NUM_13
#define tlacitko3 GPIO_NUM_14
#define ledka GPIO_NUM_32
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
#define LCD_MAX_CHARS 32 // LCD1602 has 2 lines, each can hold up to 16 characters
// Buffer to hold the last displayed text

char lcd_buffer[LCD_MAX_CHARS];
int pocetPostriku = 0;
int next_id = 1; // Globální proměnná pro sledování dalšího ID
int idStruktury = 0;
static const char *TAG = "HTTP_SERVER";
static const char *TAG4 = "JSON_Parser";
//static const char *mujTag = "MUJTAG";
static const char *TAG5 = "TIME";

bool zacaloMichani = false;
bool zahajenoPousteniVodyTlacitkem = false;
bool kvitujiFinaleMichani = false;
bool chciTarovat = false;
void isrOk(void *par);
void isrCancel(void *par);
void isrAux(void *par);
void configure_interrupt1();
void configure_interrupt2();
void configure_interrupt3();
void tare();
void test(int vaha);

typedef struct
{
    int id;
    char nazev_pripravku[50];
    char osetrovana_plodina[81];
    double mnozstvi_postriku;
    double pomer_michani;
    char doba_postriku[11];
    char denPosledniAplikacePostriku[6];
} PostrikData;


int32_t ofsetek1 = 0;
int32_t ofsetek2 = 0;
int prevodniFaktorA = 18070;
int prevodniFaktorB = 1000;
int vahaMalaVelka = 1;



#endif
