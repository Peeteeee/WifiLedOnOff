#include "main.h"

void ulozPostrikData();

PostrikData postrik_data;
PostrikData dataBazePostriku[MAX_RUZNYCH_POSTRIKU];

int delete_postrik(int id)
{
    if (id < 1)
    {
        return 0; // Neplatné ID
    }

    for (int i = 0; i < pocetPostriku; i++)
    {
        if (dataBazePostriku[i].id == id)
        {
            for (int j = i; j < pocetPostriku - 1; j++)
            {
                dataBazePostriku[j] = dataBazePostriku[j + 1];
            }
            pocetPostriku--;
        }
    }
    ulozPostrikData();

    return 1; // Úspěch
}
int generate_id()
{
    int max = 1;
    for (int i = 0; i < pocetPostriku; i++)
    {
        if (dataBazePostriku[i].id >= max)
        {
            max = dataBazePostriku[i].id + 1;
        }
    }
    return max;
}
void ulozPostrikData()
{
    FILE *soubor = fopen(FILE_PATH3, "wb");
    if (soubor == NULL)
    {
        perror("Chyba při otevírání souboru");
        return;
    }

    // Uloží počet záznamů
    fwrite(&pocetPostriku, sizeof(int), 1, soubor);

    // Uloží data
    fwrite(dataBazePostriku, sizeof(PostrikData), pocetPostriku, soubor);

    fclose(soubor);
}
void pridatPostrik(PostrikData novyPostrik)
{
    if (pocetPostriku < MAX_RUZNYCH_POSTRIKU)
    {
        novyPostrik.id = generate_id();
        dataBazePostriku[pocetPostriku] = novyPostrik;
        pocetPostriku++;
        ulozPostrikData(); // do spiffs/poleStruktur
    }
}
void nactiPostrikData()
{
    FILE *soubor = fopen(FILE_PATH3, "rb");
    if (soubor == NULL)
    {
        perror("Chyba při otevírání souboru");
        return;
    }
    fread(&pocetPostriku, sizeof(int), 1, soubor);
    fread(dataBazePostriku, sizeof(PostrikData), pocetPostriku, soubor);

    fclose(soubor);
}
int porovnejPostrikData(PostrikData *a, PostrikData *b)
{
    if (strcmp(a->nazev_pripravku, b->nazev_pripravku) != 0)
    {
        return 0;
    }
    if (strcmp(a->osetrovana_plodina, b->osetrovana_plodina) != 0)
    {
        return 0;
    }
    if (a->mnozstvi_postriku != b->mnozstvi_postriku)
    {
        return 0;
    }
    if (a->pomer_michani != b->pomer_michani)
    {
        return 0;
    }
    if (strcmp(a->doba_postriku, b->doba_postriku) != 0)
    {
        return 0;
    }
    return 1; // Struktury jsou stejné
}
void parse_json(const char *json_str, PostrikData *data)
{
    // Načtení JSON řetězce do cJSON struktury
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL)
    {
        ESP_LOGE(TAG4, "Chyba při parsování JSON");
        return;
    }

    // Získání jednotlivých hodnot
    cJSON *nazev_pripravku = cJSON_GetObjectItem(json, "pripravek");
    cJSON *osetrovana_plodina = cJSON_GetObjectItem(json, "plodina");
    cJSON *mnozstvi_postriku = cJSON_GetObjectItem(json, "mnozstvi");
    cJSON *pomer_michani = cJSON_GetObjectItem(json, "pomer");
    cJSON *doba_postriku = cJSON_GetObjectItem(json, "dobaPostriku");

    // Kontrola a zpracování hodnot
    if (cJSON_IsString(nazev_pripravku) && (nazev_pripravku->valuestring != NULL))
    {
        // Uložení názvu přípravku
        strncpy(data->nazev_pripravku, nazev_pripravku->valuestring, sizeof(data->nazev_pripravku) - 1);
        data->nazev_pripravku[sizeof(data->nazev_pripravku) - 1] = '\0'; // Null terminátor
    }
    else
    {
        ESP_LOGE(TAG4, "Chybějící nebo neplatný 'NazevPripravku'");
    }

    if (cJSON_IsString(osetrovana_plodina) && (osetrovana_plodina->valuestring != NULL))
    {
        // Uložení ošetřované plodiny
        strncpy(data->osetrovana_plodina, osetrovana_plodina->valuestring, sizeof(data->osetrovana_plodina) - 1);
        data->osetrovana_plodina[sizeof(data->osetrovana_plodina) - 1] = '\0'; // Null terminátor
    }
    else
    {
        ESP_LOGE(TAG4, "Chybějící nebo neplatný 'OsetrovanaPlodina'");
    }

    if (cJSON_IsNumber(mnozstvi_postriku))
    {
        // Uložení množství postřiku
        data->mnozstvi_postriku = mnozstvi_postriku->valuedouble;
    }
    else
    {
        ESP_LOGE(TAG4, "Chybějící nebo neplatný 'MnozstviPostriku'");
    }

    if (cJSON_IsNumber(pomer_michani))
    {
        // Uložení poměru míchání
        data->pomer_michani = pomer_michani->valuedouble;
    }
    else
    {
        ESP_LOGE(TAG4, "Chybějící nebo neplatný 'PomerMichani'");
    }

    if (cJSON_IsString(doba_postriku) && (doba_postriku->valuestring != NULL))
    {
        // Uložení doby postřiku
        strncpy(data->doba_postriku, doba_postriku->valuestring, sizeof(data->doba_postriku) - 1);
        data->doba_postriku[sizeof(data->doba_postriku) - 1] = '\0'; // Null terminátor
    }
    else
    {
        ESP_LOGE(TAG4, "Chybějící nebo neplatný 'DobaPostriku'");
    }

    // Uvolnění cJSON struktury
    cJSON_Delete(json);
}

esp_err_t uvodniStranaKsicht_handler(httpd_req_t *req)
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
esp_err_t nahlasPostriky_handler(httpd_req_t *req)
{
    // Buffer pro dočasné uchování JSON dat
    char chunk[CHUNK_SIZE];
    int len = 0;
    int offset = 0;

    // Nastavení typu odpovědi na JSON
    httpd_resp_set_type(req, "application/json");

    // Začátek JSON pole
    len = snprintf(chunk, sizeof(chunk), "[");
    if (len < 0)
    {
        ESP_LOGE("JSON", "Failed to format JSON start bracket.");
        return ESP_FAIL;
    }
    if (httpd_resp_send_chunk(req, chunk, len) != ESP_OK)
    {
        ESP_LOGE("JSON", "Failed to send JSON start bracket chunk.");
        return ESP_FAIL;
    }
    ESP_LOGI("JSON", "Sent JSON start bracket.");

    // Iterace přes každý prvek v poli dataBazePostriku
    for (int i = 0; i < pocetPostriku; i++)
    {
        // Přidání čárky, pokud není první položka
        if (i > 0)
        {
            len = snprintf(chunk, sizeof(chunk), ",");
            if (len < 0)
            {
                ESP_LOGE("JSON", "Failed to format JSON comma.");
                return ESP_FAIL;
            }
            if (httpd_resp_send_chunk(req, chunk, len) != ESP_OK)
            {
                ESP_LOGE("JSON", "Failed to send JSON comma chunk.");
                return ESP_FAIL;
            }
            ESP_LOGI("JSON", "Sent JSON comma for item %d.", i);
        }

        // Příprava JSON objektu pro aktuální položku
        len = snprintf(chunk, sizeof(chunk),
                       "{\"id\":%d,\"nazev\":\"%s\",\"plodina\":\"%s\",\"mnozstvi\":%.2f,\"pomer\":%.2f,\"doba\":\"%s\"}",
                       dataBazePostriku[i].id,
                       dataBazePostriku[i].nazev_pripravku,
                       dataBazePostriku[i].osetrovana_plodina,
                       dataBazePostriku[i].mnozstvi_postriku,
                       dataBazePostriku[i].pomer_michani,
                       dataBazePostriku[i].doba_postriku);
        if (len < 0)
        {
            ESP_LOGE("JSON", "Failed to format JSON object for item %d.", i);
            return ESP_FAIL;
        }

        // Odeslání JSON objektu po částech, pokud je příliš velký
        offset = 0;
        while (offset < len)
        {
            int chunk_len = (len - offset > CHUNK_SIZE) ? CHUNK_SIZE : (len - offset);
            if (httpd_resp_send_chunk(req, chunk + offset, chunk_len) != ESP_OK)
            {
                ESP_LOGE("JSON", "Failed to send JSON object chunk for item %d.", i);
                return ESP_FAIL;
            }
            offset += chunk_len;
            ESP_LOGI("JSON", "Sent %d bytes of JSON object chunk for item %d.", chunk_len, i);
        }
    }

    // Konec JSON pole
    len = snprintf(chunk, sizeof(chunk), "]");
    if (len < 0)
    {
        ESP_LOGE("JSON", "Failed to format JSON end bracket.");
        return ESP_FAIL;
    }
    if (httpd_resp_send_chunk(req, chunk, len) != ESP_OK)
    {
        ESP_LOGE("JSON", "Failed to send JSON end bracket chunk.");
        return ESP_FAIL;
    }
    ESP_LOGI("JSON", "Sent JSON end bracket.");

    // Indikace konce odpovědi (prázdný chunk)
    if (httpd_resp_send_chunk(req, NULL, 0) != ESP_OK)
    {
        ESP_LOGE("JSON", "Failed to send final empty chunk to indicate end of response.");
        return ESP_FAIL;
    }
    ESP_LOGI("JSON", "Completed sending JSON response.");

    return ESP_OK;
}
esp_err_t vkladaniDatKsicht_handler(httpd_req_t *req)
{

    FILE *file = fopen(FILE_PATH2, "r");
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
esp_err_t ulozDataZFormulare_handler(httpd_req_t *req)
{
    // Příprava bufferu pro příjem dat
    char buf[512];
    int ret;

    // Čtení dat z požadavku
    ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret < 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            ESP_LOGE(TAG4, "Timeout při čtení dat");
        }
        return ESP_FAIL;
    }

    // Null-terminátor pro buffer
    buf[ret] = '\0';

    // Volání funkce pro zpracování JSON
    parse_json(buf, &postrik_data);
    ESP_LOGI(TAG4, "buf:%s", buf);

    bool uzJevDatabazi = false;
    for (int i = 0; i < pocetPostriku; i++)
    {
        if (porovnejPostrikData(&dataBazePostriku[pocetPostriku - 1 - i], &postrik_data))
        {
            uzJevDatabazi = true;
            printf("alespon jeden je stejny.\n");
        }
    }
    if (!uzJevDatabazi)
    {
        printf("zadny neni stejny.\n");
        pridatPostrik(postrik_data);
    }
    ESP_LOGI(TAG4, "pocet postriku:%d", pocetPostriku);

    // Odpověď na POST požadavek
    const char resp[] = "{\"status\":\"success\"}";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
esp_err_t zmenaDatVDatabazi_handler(httpd_req_t *req)
{

    return ESP_OK;
}
esp_err_t smazaniPostriku_handler(httpd_req_t *req)
{
    // Buffer pro příjem dat
    char buf[100];
    int received = 0;
    cJSON *json = NULL;
    cJSON *id = NULL;

    // Čtení obsahu požadavku
    while (received < req->content_len)
    {
        int ret = httpd_req_recv(req, buf + received, req->content_len - received);
        if (ret < 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                continue;
            }
            ESP_LOGE(TAG, "Chyba při čtení požadavku");
            return ESP_FAIL;
        }
        received += ret;
    }
    // Parsing JSON
    buf[received] = '\0';
    printf("\n\nbuf:%s\n\n", buf);
    json = cJSON_Parse(buf);

    if (json == NULL)
    {
        ESP_LOGE(TAG, "Chyba při parsování JSON");
        // httpd_resp_send_400(req);
        return ESP_FAIL;
    }

    id = cJSON_GetObjectItem(json, "id");
    if (cJSON_IsNumber(id))
    {
        ESP_LOGI(TAG, "Mazání položky s ID = %d", id->valueint);

        delete_postrik(id->valueint);

        // Odeslání úspěšné odpovědi
        httpd_resp_send(req, "Položka byla úspěšně smazána.", HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        // Odeslání chybové odpovědi
        // httpd_resp_send_400(req);
    }

    cJSON_Delete(json);
    return ESP_OK;
}

// tabulka s prehledem vsech postriku a jejich dat
httpd_uri_t uri_uvodniStranaKsicht = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = uvodniStranaKsicht_handler,
    .user_ctx = NULL};
// tabulka na hlavni strance si zada vypis vsech hodnot vsech postriku
httpd_uri_t uri_nahlasPostriky = {
    .uri = "/postriky",
    .method = HTTP_GET,
    .handler = nahlasPostriky_handler,
    .user_ctx = NULL};
// formular pro vkladani ci upravu dat
httpd_uri_t uri_vkladaniDatKsicht = {
    .uri = "/vkladaniDat",
    .method = HTTP_GET,
    .handler = vkladaniDatKsicht_handler,
    .user_ctx = NULL};
// prijme json z formulare vkladaniDat a ulozi data do databazepostriku
httpd_uri_t uri_ulozDataZFormulare = {
    .uri = "/ulozit",
    .method = HTTP_POST,
    .handler = ulozDataZFormulare_handler,
    .user_ctx = NULL};
// zmena dat urciteho postriku. stejne okno jako /vkladanidat ale predvyplnene
httpd_uri_t uri_zmenDataVDatabazi = {
    .uri = "/zmenit",
    .method = HTTP_GET,
    .handler = zmenaDatVDatabazi_handler,
    .user_ctx = NULL};
// smazani zaznamu z databaze
httpd_uri_t uri_delete_postrik = {
    .uri = "/smazat",
    .method = HTTP_DELETE,
    .handler = smazaniPostriku_handler,
    .user_ctx = NULL};

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
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_delete_postrik);
        httpd_register_uri_handler(server, &uri_uvodniStranaKsicht);
        httpd_register_uri_handler(server, &uri_vkladaniDatKsicht);
        httpd_register_uri_handler(server, &uri_ulozDataZFormulare);
        httpd_register_uri_handler(server, &uri_nahlasPostriky);
        httpd_register_uri_handler(server, &uri_zmenDataVDatabazi);
    }
    return server;
}
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

void print_current_date(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(buffer, buffer_size, "%Y-%m-%d", &timeinfo);
}
void print_current_time(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(buffer, buffer_size, "%H:%M:%S", &timeinfo);
}
void initialize_sntp(void)
{
    ESP_LOGI(TAG5, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}
void obtain_time(void)
{
    initialize_sntp();

    // Čekání na synchronizaci času přes NTP
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 30;
    while (timeinfo.tm_year < (2022 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI(TAG5, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry == retry_count)
    {
        ESP_LOGE(TAG5, "Failed to synchronize time");
    }
    else
    {
        ESP_LOGI(TAG5, "Time synchronized successfully");
    }
}

// task na jadre 1
void michaciProcedura(void *pvParameter)
{
    while (1)
    {
        ESP_LOGI(mujTag, "michaciprocedura\n");
        if (zacinameMichat)
        {
            zacinameMichat = false; // muzu resumovat tlacitkem
            double potrebneMnozstviPripravku = postrik_data.mnozstvi_postriku * postrik_data.pomer_michani;
            // tare mala vaha
            //  displej: vyber pripravek "pripravek"
            //  nasyp na misku potrebneMnozstviPripravku

            // while(potrebneMnozstviPripravku < funkceVaha...jeste vymyslim)
            {
                // displej ukazuje zbyvajici hmotnost
            }
            // displej: hotovo
            // displej: potvrd spusteni vody
            while (nezahajenoPousteniVodyTlacitkem)
            {
                ESP_LOGI(mujTag, "cekam na svoleni ke pusteni vody:\n");
                ESP_LOGI(mujTag, "potrebne mnozstvi pripravku:%f\n", potrebneMnozstviPripravku);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            ESP_LOGI(mujTag, "poustim vodu\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void mujTaskNaJadreJedna(void *pvParameter)
{
    nactiPostrikData();
    vTaskDelay(10 / portTICK_PERIOD_MS);

    while (1)
    {
        for (int i = 0; i < pocetPostriku; i++)
        {
            // printf("postrik nazev:%s\n", dataBazePostriku[i].nazev_pripravku);
            // printf("postrik plodina:%s\n", dataBazePostriku[i].osetrovana_plodina);
            // printf("postrik mnozstvi:%f\n", dataBazePostriku[i].mnozstvi_postriku);
            // printf("postrik pomer:%f\n", dataBazePostriku[i].pomer_michani);
            // printf("postrik datum:%s\n", dataBazePostriku[i].doba_postriku);
            // printf("postrik id:%d\n", dataBazePostriku[i].id);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        printf("\npocet postriku:%d\n\n", pocetPostriku);
        lcd1602_clear();
        lcd1602_write("Postriku: ");
        static char str[8];                              // Velikost dostatečná pro int (včetně záporného znaménka a null-terminátoru)
        snprintf(str, sizeof(str), "%d", pocetPostriku); // Převod integeru na řetězec
        lcd1602_write(str);
        // vTaskDelay(500/portTICK_PERIOD_MS);
        lcd1602_set_cursor(1, 0);
        //lcd1602_write("Lena is the best!");

char textProDisplej[10];
print_current_time(textProDisplej, sizeof(textProDisplej));
lcd1602_write(textProDisplej);


        char date_str[64];
        print_current_date(date_str, sizeof(date_str));

        if (strcmp(date_str, postrik_data.doba_postriku) == 0)
        {
            // zacinameMichat = true;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(mujTag, "JeduPrvniJadroTask\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();}
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();
    obtain_time();
    init_spiffs();
    start_webserver();
    lcd1602_init(RS, EN, D4, D5, D6, D7);
    lcd1602_shift_left();
    //gpio_set_direction(relePin, GPIO_MODE_OUTPUT);
    //gpio_set_level(relePin, 0);
    
    xTaskCreatePinnedToCore(
        mujTaskNaJadreJedna,   // Task function
        "mujTaskNaJadreJedna", // Task name
        2048,                  // Stack size
        NULL,                  // Parameters
        1,                     // Priority
        NULL,                  // Task handle
        1                      // Core ID (0 or 1)
    );
    xTaskCreatePinnedToCore(
        michaciProcedura,   // Task function
        "michaciProcedura", // Task name
        4096,               // Stack size
        NULL,               // Parameters
        5,                  // Priority
        NULL,               // Task handle
        1                   // Core ID (0 or 1)
    );
}