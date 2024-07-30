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

static const char *TAG = "HTTP_SERVER";
static uint8_t led_state = 0;
#define relePin GPIO_NUM_2

// WiFi SSID a heslo
#define EXAMPLE_ESP_WIFI_SSID "TP-LINK_0D7A"
#define EXAMPLE_ESP_WIFI_PASS "cabracina128"

esp_err_t get_handler(httpd_req_t *req)
{
    const char on_resp[] = "<h1>ON:</h1>"
                           "<a href=\"/off\">Off</a>";
    const char off_resp[] = "<h1>OFF:</h1>"
                            "<a href=\"/on\">On</a>";

    if (led_state == 0)
        httpd_resp_send(req, off_resp, HTTPD_RESP_USE_STRLEN);
    else
        httpd_resp_send(req, on_resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t on(httpd_req_t *req)
{
    gpio_set_level(relePin, 1);
    led_state = 1;
    const char resp[] = "<h1>ON:</h1>"
                        "<a href=\"/off\">Off</a>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t off(httpd_req_t *req)
{
    gpio_set_level(relePin, 0);
    led_state = 0;
    const char resp[] = "<h1>OFF:</h1>"
                        "<a href=\"/on\">On</a>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_on = {
    .uri = "/on",
    .method = HTTP_GET,
    .handler = on,
    .user_ctx = NULL};

httpd_uri_t uri_off = {
    .uri = "/off",
    .method = HTTP_GET,
    .handler = off,
    .user_ctx = NULL};


// Start HTTP server
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_on);
        httpd_register_uri_handler(server, &uri_off);
    }

    return server;
}

// Event handler for WiFi events
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        //ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        //ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
    }
}

void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &event_handler,
                                        NULL,
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &event_handler,
                                        NULL,
                                        &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void app_main(void)
{

    gpio_set_direction(relePin, GPIO_MODE_OUTPUT);
    gpio_set_level(relePin, 0);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // Start the web server
    start_webserver();
}
