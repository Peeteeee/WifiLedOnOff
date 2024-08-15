#ifndef __MAIN_H__
#define __MAIN_H__

#include "cJSON.h"
#include "driver/gpio.h"
#include <driver/i2c.h>
#include "esp_event.h"
#include "esp_http_server.h"
#include <esp_log.h>
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "HD44780.h"
#include <hx711.h>
#include <inttypes.h>
#include "lwip/inet.h"
#include <math.h>
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <time.h>
// #define RS GPIO_NUM_2
// #define EN GPIO_NUM_4
// #define D4 GPIO_NUM_5
// #define D5 GPIO_NUM_21
// #define D6 GPIO_NUM_22
// #define D7 GPIO_NUM_23
#define IN GPIO_NUM_27
#define INH GPIO_NUM_33
// 12, 13, 14, 15 ... JTAG
#define tlacitko1 GPIO_NUM_25//tlacitko1
#define tlacitko2 GPIO_NUM_26//tlacitko2
#define ledka GPIO_NUM_32//led1
#define FILE_PATH "/spiffs/uvodniStrana.html"
#define FILE_PATH2 "/spiffs/vkladaniDat.html"
#define FILE_PATH3 "/spiffs/zmenaDat.html"
#define FILE_PATH4 "/spiffs/poleStruktur.bin"

#define MAX_RUZNYCH_POSTRIKU 50
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"
#define MAX_DATA_LENGTH 256
#define CHUNK_SIZE 1024
#define LCD_MAX_CHARS 81

#define LCD_ADDR 0x27
#define SDA_PIN 21
#define SCL_PIN 22
#define LCD_COLS 20
#define LCD_ROWS 4

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
static const char *TAG2 = "TASK_MUJ_TASK";
static const char *TAG3 = "TASK_MICHANI";
static const char *TAG4 = "JSON_Parser";
static const char *TAG5 = "TIME";
static const char *TAG6 = "ulozDataZFormulare_handler";
static const char *TAG7 = "prevodDatumu";
static const char *TAG8 = "Predvyplneni";
static const char *TAG9 = "nactiPostrikData";
static const char *TAG10 = "TARE";
static const char *TAG11 = "zvazPripravek";
static const char *TAG12 = "seradDatabaziPodleData";
static const char *TAG13 = "smazaniPostriku_handler";
static const char *TAG14 = "zmenaDatVDatabaziId_handler";
static const char *TAG15 = "extract_id_from_json";
//static const char *TAG16 = "";
static const char *TAG17 = "aktualizujDenAplikace";
static const char *TAG18 = "delete_postrik";
static const char *TAG19 = "generate_id";
//static const char *TAG20 = "mamNecoKmichani";
static const char *TAG21 = "pridatPostrik";
static const char *TAG22 = "zpracujPostrikData";
static const char *TAG23 = "nahlasPostriky_handler";
static const char *TAG24 = "cekejNaFinalizaciMichani";
static const char *TAG25 = "jePostrikVdatabazi";
bool probihaMichani = false;
bool zahajenoPousteniVodyTlacitkem = false;
bool kvitujiFinaleMichani = false;
bool chciTarovat = false;
bool uzJevDatabazi = false;
bool mamNecoKmichanipromenna = false;
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

void aktualizujDenAplikace(int idStruktury, const char *denAplikace);
void cekejNaFinalizaciMichani(char *osetrovanaPlodina);
void cekejNaSpusteniVody();
int delete_postrik(int id);
bool extract_id_from_json(cJSON *json, int *id);
int generate_id();
bool jePostrikVdatabazi(PostrikData *postrik_data);
void konfiguraceTimeru(void);
void kontrola_kmichani_cb();
bool lcd_update(const char *text, int line);
bool mamNecoKmichani(void);
void nactiPostrikData();
void napustVodu(double litru);
void parse_json_for_id(const char *json_str, PostrikData *data);
void parse_json(const char *json_str, PostrikData *data);
int porovnejDatumy(const char *datum1, const char *datum2);
int porovnejPostrikData(PostrikData *a, PostrikData *b);
void prevodDatumu(char *datum);
void pridatPostrik(PostrikData novyPostrik);
void print_current_date(char *buffer, size_t buffer_size);
void print_current_time(char *buffer, size_t buffer_size);
void seradDatabaziPodleData();
void tare();
void ulozPostrikData();
int vratPoradoveCisloStrukturyVpoli(int id);
void zpracujPostrikData(int idStruktury, double *mnozstviPostriku, double *pomerMichani, char *nazevPripravku, char *osetrovanaPlodina);
void zvazPripravek(double gramu);

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
SemaphoreHandle_t mutexPostriku;

PostrikData postrik_data = {.id = 0, .mnozstvi_postriku = 0.0, .pomer_michani = 0.0, .denPosledniAplikacePostriku = "02-02"};
PostrikData dataBazePostriku[MAX_RUZNYCH_POSTRIKU];

#endif
