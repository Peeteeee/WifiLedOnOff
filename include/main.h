#ifndef __MAIN_H__
#define __MAIN_H__

#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include <esp_log.h>
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <hx711.h>
#include <inttypes.h>
#include <lcd1602.h>
#include "lwip/inet.h"
#include <math.h>
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <time.h>
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
#define FILE_PATH3 "/spiffs/zmenaDat.html"
#define FILE_PATH4 "/spiffs/poleStruktur.bin"

#define MAX_RUZNYCH_POSTRIKU 20
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"
#define MAX_DATA_LENGTH 256
#define CHUNK_SIZE 1024
#define LCD_MAX_CHARS 32

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

char lcd_buffer[LCD_MAX_CHARS];
int idStruktury = 0;
int idStruktury2 = 0;
int next_id = 1;
int pocetPostriku = 0;
int32_t ofsetek1 = 0;
int32_t ofsetek2 = 0;
int prevodniFaktorA = 18070;
int prevodniFaktorB = 1000;

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

void wifi_init_sta(void);
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void init_spiffs(void);
void initialize_sntp(void);
void obtain_time(void);
static httpd_handle_t start_webserver(void);

int delete_postrik(int id);
bool extract_id_from_json(cJSON *json, int *id);
int generate_id();
bool lcd_update(const char *text, int line);
void nactiPostrikData();
void napustVodu(double litru);
void parse_json_for_id(const char *json_str, PostrikData *data);
void parse_json(const char *json_str, PostrikData *data);
int porovnejPostrikData(PostrikData *a, PostrikData *b);
void prevodDatumu(char *datum);
void pridatPostrik(PostrikData novyPostrik);
void print_current_date(char *buffer, size_t buffer_size);
void print_current_time(char *buffer, size_t buffer_size);
void seradDatabaziPodleData();
void ulozPostrikData();
void tare();



esp_err_t uvodniStranaKsicht_handler(httpd_req_t *req);
esp_err_t nahlasPostriky_handler(httpd_req_t *req);
esp_err_t vkladaniDatKsicht_handler(httpd_req_t *req);
esp_err_t ulozDataZFormulare_handler(httpd_req_t *req);
esp_err_t zmenaDatVDatabaziKsicht_handler(httpd_req_t *req);
esp_err_t zmenaDatVDatabaziId_handler(httpd_req_t *req);
esp_err_t zmenDataVDatabaziPredvyplneni_handler(httpd_req_t *req);
esp_err_t smazaniPostriku_handler(httpd_req_t *req);

SemaphoreHandle_t Displej;
SemaphoreHandle_t Tlacitko1;
SemaphoreHandle_t Tlacitko2;
SemaphoreHandle_t Tlacitko3;

PostrikData postrik_data = {.id = 0, .mnozstvi_postriku = 0.0, .pomer_michani = 0.0, .denPosledniAplikacePostriku = "02-02"};
PostrikData dataBazePostriku[MAX_RUZNYCH_POSTRIKU];

#endif
