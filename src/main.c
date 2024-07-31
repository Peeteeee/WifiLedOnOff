#include "main.h"
char *cti_druh_postriku()
{
    FILE *file = fopen(FILE_PATH2, "r");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file for reading: %s", FILE_PATH2);
        return NULL;
    }

    // Získání velikosti souboru
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Alokace paměti pro obsah souboru
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for file content");
        fclose(file);
        return NULL;
    }

    // Čtení souboru do bufferu
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size)
    {
        ESP_LOGE(TAG, "Failed to read the complete file content");
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[file_size] = '\0'; // Přidání nulového terminátoru

    fclose(file);
    return buffer;
}
void init_spiffs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    // Inicializace SPIFFS
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE("SPIFFS", "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE("SPIFFS", "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE("SPIFFS", "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI("SPIFFS", "Partition size: total: %d, used: %d", total, used);
    }
}

esp_err_t get_handler(httpd_req_t *req)
{
    FILE *file = fopen(FILE_PATH, "r");
    if (file == NULL)
    {
        ESP_LOGE("FILE", "Failed to open file for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Získání velikosti souboru
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Alokace paměti pro obsah souboru
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL)
    {
        ESP_LOGE("FILE", "Failed to allocate memory for file content");
        fclose(file);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Čtení souboru do bufferu
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0'; // Přidání nulového terminátoru

    fclose(file);

    // Odeslání obsahu souboru jako HTTP odpověď
    httpd_resp_set_type(req, "text/html; charset=UTF-8");

    httpd_resp_send(req, buffer, file_size);

    free(buffer);
    return ESP_OK;
}
esp_err_t on(httpd_req_t *req)
{
    gpio_set_level(relePin, 1);
    led_state = 1;

    // Nastavení hlavičky Content-Type na UTF-8
    httpd_resp_set_type(req, "text/html; charset=UTF-8");

    const char resp[] =
        "<h1>ON:</h1>"
        "<form action=\"/submit\" method=\"post\">"
        "Druh postřiku: <input type=\"text\" name=\"druh_postriku\" />"
        "<input type=\"submit\" value=\"Submit\" />"
        "</form>"
        "<a href=\"/off\">Off</a>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
esp_err_t off(httpd_req_t *req)
{
    gpio_set_level(relePin, 0);
    led_state = 0;

    // Načtení hodnoty ze souboru
    char *druhPostriku = cti_druh_postriku();
    // Použití načtené hodnoty ve vašem programu
    ESP_LOGI(TAG2, "Loaded value: %s", druhPostriku);
    // Například použití hodnoty k nějaké logice
    free(druhPostriku);

    const char resp[] = "<h1>OFF:</h1>"
                        "<a href=\"/on\">On</a>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Funkce pro zpracování POST požadavku a uložení do SPIFFS
esp_err_t handle_post(httpd_req_t *req)
{
    char buf[100];
    int ret = 0; // Inicializace ret
    int remaining = req->content_len;

    // Přečtěte data z požadavku
    while (remaining > 0)
    {
        ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }

    // Přidejte nulový terminátor na konec přijatého řetězce
    buf[ret] = '\0';

    // Vyhledejte hodnotu parametru "druh_postriku"
    char *param = strstr(buf, "druh_postriku=");
    if (param != NULL)
    {
        param += strlen("druh_postriku=");

        // Pokud se parameter nachází, získáme jeho hodnotu
        char *end = strchr(param, '&');
        if (end != NULL)
        {
            *end = '\0';
        }

        // Ošetřete případ, kdy by hodnota mohla obsahovat URL zakódované znaky
        char decoded_value[100];
        int i, j;
        for (i = 0, j = 0; param[i]; i++)
        {
            if (param[i] == '%')
            {
                int val;
                sscanf(&param[i + 1], "%2x", &val);
                decoded_value[j++] = (char)val;
                i += 2;
            }
            else if (param[i] == '+')
            {
                decoded_value[j++] = ' ';
            }
            else
            {
                decoded_value[j++] = param[i];
            }
        }
        decoded_value[j] = '\0';

        // Nyní můžeme uložit `decoded_value` do SPIFFS
        FILE *f = fopen(FILE_PATH2, "w");
        if (f == NULL)
        {
            ESP_LOGE("SPIFFS", "Failed to open file for writing");
            return ESP_FAIL;
        }
        fprintf(f, "%s", decoded_value);
        fclose(f);

        ESP_LOGI("SPIFFS", "File written: %s", decoded_value);
    }

    // Odpovězte uživateli
    // Nastavení hlavičky Content-Type na UTF-8
    httpd_resp_set_type(req, "text/html; charset=UTF-8");

    const char response[] = "<h1>Data přijata a uložena</h1><a href=\"/\">Back</a>";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

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
httpd_uri_t uri_post = {
    .uri = "/submit",
    .method = HTTP_POST,
    .handler = handle_post,
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
        // Přidejte tuto URI do seznamu URI handlerů
        httpd_register_uri_handler(server, &uri_post);
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
        // ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        // ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
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
    // Inicializace SPIFFS
    init_spiffs();
    // Start the web server
    start_webserver();

    // // Načtení hodnoty ze souboru
    // char *druhPostriku = cti_druh_postriku();
    // // Použití načtené hodnoty ve vašem programu
    // ESP_LOGI(TAG2, "Loaded value: %s", druhPostriku);

    // // Například použití hodnoty k nějaké logice

    // free(druhPostriku);
}