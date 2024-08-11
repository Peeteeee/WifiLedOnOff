#include "main.h"

void isrOk(void *par)
{
    gpio_set_level(ledka, 1);
    zahajenoPousteniVodyTlacitkem = true;
}
void isrCancel(void *par)
{
    gpio_set_level(ledka, 0);
    zacaloMichani = true;
}
void isrAux(void *par)
{
    kvitujiFinaleMichani = true;
}
void configure_interrupt1()
{
    gpio_set_direction(tlacitko1, GPIO_MODE_INPUT);
    gpio_set_intr_type(tlacitko1, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(tlacitko1, isrOk, NULL);
}
void configure_interrupt2()
{
    gpio_set_direction(tlacitko2, GPIO_MODE_INPUT);
    gpio_set_intr_type(tlacitko2, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(tlacitko2, isrCancel, NULL);
}
void configure_interrupt3()
{
    gpio_set_direction(tlacitko3, GPIO_MODE_INPUT);
    gpio_set_intr_type(tlacitko3, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(tlacitko3, isrAux, NULL);
}

int delete_postrik(int id)
{
    // Zkontrolovat, zda je poskytnuté ID platné
    if (id < 1 || pocetPostriku == 0)
    {
        return 0; // Vrátit 0 pro označení neplatného ID nebo prázdného seznamu
    }

    // Procházet pole postřikových dat
    for (int i = 0; i < pocetPostriku; i++)
    {
        // Zkontrolovat, zda ID aktuálního postřiku odpovídá danému ID
        if (dataBazePostriku[i].id == id)
        {
            // Posunout všechny prvky po nalezeném prvku o jednu pozici vlevo
            for (int j = i; j < pocetPostriku - 1; j++)
            {
                dataBazePostriku[j] = dataBazePostriku[j + 1];
            }
            // Snížit počet postřikových záznamů
            pocetPostriku--;

            // Zavolat funkci pro uložení aktualizovaných postřikových dat
            ulozPostrikData();

            // Předčasně ukončit smyčku po úspěšném smazání
            break;
        }
    }

    // Vrátit 1 pro označení úspěchu
    return 1;
}
bool extract_id_from_json(cJSON *json, int *id)
{
    cJSON *id_item = cJSON_GetObjectItem(json, "id");

    if (id_item == NULL)
    {
        ESP_LOGE(TAG4, "Položka 'id' nebyla nalezena v JSON objektu");
        return false;
    }

    if (!cJSON_IsNumber(id_item))
    {
        ESP_LOGE(TAG4, "Položka 'id' není typu číslo");
        return false;
    }

    *id = id_item->valueint;
    ESP_LOGI(TAG4, "Úspěšně načteno id: %d", *id);
    return true;
}
int generate_id()
{
    uint16_t max = 1; // Inicializujeme max na 0, což umožní správnou funkci i pro prázdnou databázi.

    // Iterujeme přes všechny záznamy v databázi postřiků
    for (int i = 0; i < pocetPostriku; i++)
    {
        // Pokud je aktuální ID větší nebo rovno max, aktualizujeme max na (ID + 1)
        if (dataBazePostriku[i].id >= max)
        {
            max = dataBazePostriku[i].id + 1;
        }
    }

    // Vracíme nově generované ID, které je nejvyšší existující ID + 1
    return max;
}
bool lcd_update(const char *text, int line)
{
    int offset = line * 16;
    char retezec[17];

    // Check if the text is already displayed
    if (strncmp(&lcd_buffer[offset], text, 16) == 0)
    {
        // ESP_LOGI(TAG, "Text is already displayed on line %d: %s", line, text);
        return false;
    }

    // ESP_LOGI(TAG, "Updating line %d with new text: %s", line, text);

    // Clear the line before writing new text
    lcd1602_set_cursor(line, 0); // Set cursor to the beginning of the line
    // ESP_LOGI(TAG, "Clearing line %d", line);
    lcd1602_write("                "); // Clear the line with spaces

    // Write new text to the display
    lcd1602_set_cursor(line, 0);                    // Set cursor to the beginning of the line again
    snprintf(retezec, sizeof(retezec), "%s", text); // Copy the text to a temporary buffer
    // ESP_LOGI(TAG, "Writing new text to line %d: %s", line, retezec);
    lcd1602_write(retezec);

    // Update buffer
    strncpy(&lcd_buffer[offset], text, 16);

    // Null-terminate the rest of the buffer if the text is shorter than 16 characters
    for (int i = strlen(text); i < 16; i++)
    {
        lcd_buffer[offset + i] = '\0';
    }

    // ESP_LOGI(TAG, "Buffer updated for line %d: %s", line, &lcd_buffer[offset]);

    return true;
}
bool mamNecoKmichani(void)
{
    char datumTed[64];
    char datumPostriku[11];
    char denAplikace[64];

    // Get the current date
    print_current_date(datumTed, sizeof(datumTed));

    for (int i = 0; i < pocetPostriku; i++)
    {
        // Copy data from the spray database
        strcpy(datumPostriku, dataBazePostriku[i].doba_postriku);
        strcpy(denAplikace, dataBazePostriku[i].denPosledniAplikacePostriku);

        // Assuming prevodDatumu converts the date to a specific format
        prevodDatumu(datumPostriku);

        // Log the current state of the variables
        // printf("Kontrola postřiku %d:\n", i + 1);
        // printf(" - Datum postřiku: %s\n", datumPostriku);
        // printf(" - Den poslední aplikace: %s\n", denAplikace);

        // Condition to check if the spray needs mixing
        if ((strcmp(datumTed, datumPostriku) == 0) && (strcmp(datumTed, denAplikace) != 0))
        {
            idStruktury = dataBazePostriku[i].id;
            return true;
        }
    }
    // printf("Žádný postřik k míchání nenalezen.\n");
    return false;
}
void nactiPostrikData()
{
    FILE *soubor = fopen(FILE_PATH4, "rb");
    if (soubor == NULL)
    {
        perror("Chyba při otevírání souboru");
        return;
    }
    fread(&pocetPostriku, sizeof(int), 1, soubor);
    fread(dataBazePostriku, sizeof(PostrikData), pocetPostriku, soubor);

    fclose(soubor);
    seradDatabaziPodleData();
}
void napustVodu(double litru)
{
    for (int i = 0; i < 30; i++)
    {
        printf("\nNapoustim vodu:%d\n", i);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    zahajenoPousteniVodyTlacitkem = false;
}
void parse_json_for_id(const char *json_str, PostrikData *data)
{
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL)
    {
        ESP_LOGE(TAG4, "Chyba při parsování JSON: Neplatný JSON řetězec");
        return;
    }

    if (!extract_id_from_json(json, &data->id))
    {
        ESP_LOGE(TAG4, "Nepodařilo se získat platné ID z JSON");
    }

    cJSON_Delete(json);
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
void prevodDatumu(char *datum)
{
    // 2024-08-05
    // 05-08
    char datumKuprave[6];
    datumKuprave[0] = datum[8];
    datumKuprave[1] = datum[9];
    datumKuprave[2] = '-';
    datumKuprave[3] = datum[5];
    datumKuprave[4] = datum[6];
    datumKuprave[5] = '\0';
    strcpy(datum, datumKuprave);
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
void print_current_date(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(buffer, buffer_size, "%d-%m", &timeinfo);
}
void print_current_time(char *buffer, size_t buffer_size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(buffer, buffer_size, "%H:%M:%S", &timeinfo);
}
void seradDatabaziPodleData()
{
    printf("datum postriku v serad databazi:%s\n", dataBazePostriku[0].doba_postriku);
    //     for (int i = 0; i < pocetPostriku - 1; i++)
    //     {
    //         for (int j = 1; j < pocetPostriku; j++)
    //         {
    //             //dd.mm
    //             if (dataBazePostriku[i].doba_postriku[4] > dataBazePostriku[j].doba_postriku[4])
    //             {

    //             }
    //         }
    //     }
}
void ulozPostrikData()
{
    FILE *soubor = fopen(FILE_PATH4, "wb");
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
void tare()
{
    hx711_t dev = {
        .dout = 19,
        .pd_sck = 18,
        .gain = HX711_GAIN_A_128};

    ESP_ERROR_CHECK(hx711_init(&dev));

    esp_err_t r = hx711_wait(&dev, 800); // 200
    if (r != ESP_OK)
    {
        ESP_LOGE(TAG, "Device not found... tarefce: %d (%s)\n", r, esp_err_to_name(r));
    }
    int32_t data = 0;
    r = hx711_read_median(&dev, 20, &data); // musi byt sude
    if (r != ESP_OK)
    {
        ESP_LOGE(TAG, "Could not read data... tarefce: %d (%s)\n", r, esp_err_to_name(r));
    }
    ofsetek1 = data;
    chciTarovat = false;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
void zvazPripravek(double gramu)
{
    ESP_LOGI(TAG, "Starting weighing process for %f grams", gramu); // Log start of function
    printf("zvaz:gramu:%f\n", gramu);                               // Console output for debugging

    // Initialize the HX711 device structure
    hx711_t dev = {
        .dout = 19,
        .pd_sck = 18,
        .gain = HX711_GAIN_A_128};

    // Initialize device and check for errors
    ESP_ERROR_CHECK(hx711_init(&dev));
    tare();
    ESP_LOGI(TAG, "Device initialized and tared");

    bool jesteToNeniDost = true; // Variable to control the weighing loop
    int32_t data = 0;            // Variable to store raw data from HX711

    while (jesteToNeniDost)
    {
        // Read median value from HX711
        esp_err_t r = hx711_read_median(&dev, 6, &data);
        ESP_LOGE(TAG, "Merim %ld g\n", data);
        ESP_LOGE(TAG, "Chci %f g\n", gramu);
        if (r != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not read data... error: %d (%s)", r, esp_err_to_name(r));
            continue; // Skip the rest of the loop if data read fails
        }

        // Calculate the actual weight
        double novaData = (double)data - ofsetek1;
        double dataProDisplay = novaData / prevodniFaktorA < 0 ? -novaData / prevodniFaktorA : novaData / prevodniFaktorA;
        ESP_LOGE(TAG, "Merim %f g\n", dataProDisplay);
        ESP_LOGE(TAG, "Chci %f g\n", gramu);

        // Format the weight data as a string
        char stringKzobrazeni[10];                                                    // Buffer to hold the string representation of the weight
        snprintf(stringKzobrazeni, sizeof(stringKzobrazeni), "%.2f", dataProDisplay); // Safe string formatting

        // Update the display with the current weight
        if (xSemaphoreTake(Displej, portMAX_DELAY))
        {
            lcd_update(stringKzobrazeni, 0);
            lcd_update("Syp, syp.", 1);
            xSemaphoreGive(Displej);
            ESP_LOGI(TAG, "Display updated with weight: %s", stringKzobrazeni);
        }
        else
        {
            ESP_LOGW(TAG, "Failed to take semaphore for display update");
        }

        // Check if the desired weight is reached
        if (dataProDisplay > gramu)
        {
            jesteToNeniDost = false;
            ESP_LOGI(TAG, "Desired weight reached: %f grams", dataProDisplay);
        }
    }
    ESP_LOGI(TAG, "Weighing process completed");
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
                       "{\"id\":%d,\"nazev\":\"%s\",\"plodina\":\"%s\",\"mnozstvi\":%.2f,\"pomer\":%.2f,\"doba\":\"%s\",\"posledni\":\"%s\"}",
                       dataBazePostriku[i].id,
                       dataBazePostriku[i].nazev_pripravku,
                       dataBazePostriku[i].osetrovana_plodina,
                       dataBazePostriku[i].mnozstvi_postriku,
                       dataBazePostriku[i].pomer_michani,
                       dataBazePostriku[i].doba_postriku,
                       dataBazePostriku[i].denPosledniAplikacePostriku);
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
        }
    }
    if (!uzJevDatabazi)
    {
        pridatPostrik(postrik_data);
    }
    ESP_LOGI(TAG4, "pocet postriku:%d", pocetPostriku);
    bool jsouDvaStejny = false;
    for (int x = 0; x < pocetPostriku - 1; x++)
    {
        for (int y = 1; y < pocetPostriku; y++)
        {

            if (
                dataBazePostriku[x].nazev_pripravku == dataBazePostriku[y].nazev_pripravku &&
                dataBazePostriku[x].osetrovana_plodina == dataBazePostriku[y].osetrovana_plodina &&
                dataBazePostriku[x].mnozstvi_postriku == dataBazePostriku[y].mnozstvi_postriku &&
                dataBazePostriku[x].pomer_michani == dataBazePostriku[y].pomer_michani &&
                dataBazePostriku[x].doba_postriku == dataBazePostriku[y].doba_postriku)
            {
                jsouDvaStejny = true;
            }
        }
    }
    if (idStruktury2 != 0 && pocetPostriku > 1 && !uzJevDatabazi)
    {
        delete_postrik(idStruktury2);
    }
    idStruktury2 = 0;
    // Odpověď na POST požadavek
    const char resp[] = "{\"status\":\"success\"}";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
esp_err_t zmenaDatVDatabaziKsicht_handler(httpd_req_t *req)
{
    FILE *file = fopen(FILE_PATH3, "r");
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

    httpd_resp_set_type(req, "text/html; charset=UTF-8");
    httpd_resp_send(req, buffer, file_size);
    free(buffer);
    return ESP_OK;
}
esp_err_t zmenaDatVDatabaziId_handler(httpd_req_t *req)
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

    ESP_LOGI(TAG4, "buf ve fci zmena dat:%s\n", buf);
    // Volání funkce pro zpracování JSON
    parse_json_for_id(buf, &postrik_data);
    idStruktury2 = postrik_data.id;

    // Odpověď na POST požadavek
    const char resp[] = "{\"status\":\"success\"}";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}
esp_err_t zmenDataVDatabaziPredvyplneni_handler(httpd_req_t *req)
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

    // Příprava JSON objektu pro aktuální položku

    for (int j = 0; j < pocetPostriku; j++)
    {
        if (dataBazePostriku[j].id == idStruktury2)
        {
            len = snprintf(chunk, sizeof(chunk), "{\"nazev\":\"%s\",\"plodina\":\"%s\",\"mnozstvi\":%.2f,\"pomer\":%.2f,\"doba\":\"%s\"}",

                           dataBazePostriku[j].nazev_pripravku,
                           dataBazePostriku[j].osetrovana_plodina,
                           dataBazePostriku[j].mnozstvi_postriku,
                           dataBazePostriku[j].pomer_michani,
                           dataBazePostriku[j].doba_postriku);
        }
    }
    if (len < 0)
    {
        ESP_LOGE("JSON", "Failed to format JSON object for item %d.", idStruktury2);
        return ESP_FAIL;
    }
    printf("\nchunk z vyplnenihandler%s\n", chunk);
    // Odeslání JSON objektu po částech, pokud je příliš velký
    offset = 0;
    while (offset < len)
    {
        int chunk_len = (len - offset > CHUNK_SIZE) ? CHUNK_SIZE : (len - offset);
        if (httpd_resp_send_chunk(req, chunk + offset, chunk_len) != ESP_OK)
        {
            ESP_LOGE("JSON", "Failed to send JSON object chunk for item %d.", idStruktury2);
            return ESP_FAIL;
        }
        offset += chunk_len;
        ESP_LOGI("JSON", "Sent %d bytes of JSON object chunk for item %d.", chunk_len, idStruktury2);
    }
    //}

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

httpd_uri_t uri_uvodniStranaKsicht = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = uvodniStranaKsicht_handler,
    .user_ctx = NULL};
httpd_uri_t uri_nahlasPostriky = {
    .uri = "/postriky",
    .method = HTTP_GET,
    .handler = nahlasPostriky_handler,
    .user_ctx = NULL};
httpd_uri_t uri_vkladaniDatKsicht = {
    .uri = "/vkladaniDat",
    .method = HTTP_GET,
    .handler = vkladaniDatKsicht_handler,
    .user_ctx = NULL};
httpd_uri_t uri_ulozDataZFormulare = {
    .uri = "/ulozit",
    .method = HTTP_POST,
    .handler = ulozDataZFormulare_handler,
    .user_ctx = NULL};
httpd_uri_t uri_zmenDataVDatabaziKsicht = {
    .uri = "/zmenit",
    .method = HTTP_GET,
    .handler = zmenaDatVDatabaziKsicht_handler,
    .user_ctx = NULL};
httpd_uri_t uri_zmenDataVDatabaziId = {
    .uri = "/zmenit2",
    .method = HTTP_POST,
    .handler = zmenaDatVDatabaziId_handler,
    .user_ctx = NULL};
httpd_uri_t uri_zmenDataVDatabaziPredvyplneni = {
    .uri = "/ziskatData",
    .method = HTTP_GET,
    .handler = zmenDataVDatabaziPredvyplneni_handler,
    .user_ctx = NULL};
httpd_uri_t uri_delete_postrik = {
    .uri = "/smazat",
    .method = HTTP_DELETE,
    .handler = smazaniPostriku_handler,
    .user_ctx = NULL};

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
    const int retry_count = 50;
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
        httpd_register_uri_handler(server, &uri_zmenDataVDatabaziKsicht);
        httpd_register_uri_handler(server, &uri_zmenDataVDatabaziId);
        httpd_register_uri_handler(server, &uri_zmenDataVDatabaziPredvyplneni);
    }
    return server;
}

void michaciProcedura(void *pvParameter)
{
    while (1)
    {
        // printf("\nmichaci procedura, zatim nemicham\n");
        if (zacaloMichani)
        {
            double mnozstviPostriku = 0;
            double pomerMichani = 0;
            double potrebneMnozstviPripravku = 0;
            char nazevPripravku[50];
            char osetrovanaPlodina[81];
            printf("\nmichaci procedura, uz micham1\n");
            if (xSemaphoreTake(Displej, portMAX_DELAY))
            {
                for (int i = 0; i < pocetPostriku; i++)
                {
                    if (dataBazePostriku[i].id == idStruktury)
                    {
                        mnozstviPostriku = dataBazePostriku[i].mnozstvi_postriku;
                        pomerMichani = dataBazePostriku[i].pomer_michani;
                        strcpy(nazevPripravku, dataBazePostriku[i].nazev_pripravku);
                        strcpy(osetrovanaPlodina, dataBazePostriku[i].osetrovana_plodina);
                    }
                }
                potrebneMnozstviPripravku = mnozstviPostriku * pomerMichani;
                printf("\nmichaci procedura, uz micham5\n");

                // displej: potvrd spusteni vody
                char mnozstviPripravku[20];
                sprintf(mnozstviPripravku, "%.2f", potrebneMnozstviPripravku);
                printf("\nmichaci procedura, uz micham6\n");
                char str[] = "Mnozstvi:";
                printf("\nmichaci procedura, uz micham6.1\n");
                strcat(str, mnozstviPripravku);

                // char ch = 'g';
                // strcat(str, &ch);

                printf("\nmichaci procedura, uz micham7\n");

                lcd_update(str, 0);
                lcd_update(" ", 1);
                xSemaphoreGive(Displej);
                printf("\nmichaci procedura, uz micham8\n");
            }
            ESP_LOGE(TAG, "ve fci michiciprocedura Chci %f g\n", potrebneMnozstviPripravku);

            zvazPripravek(potrebneMnozstviPripravku);
            if (xSemaphoreTake(Displej, portMAX_DELAY))
            {

                while (!zahajenoPousteniVodyTlacitkem)
                {
                    lcd_update("Pustit vodu?", 0);
                    lcd_update("Tlacitko 1", 1);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                if (zahajenoPousteniVodyTlacitkem)
                {
                    napustVodu(1.4);
                }
                while (!kvitujiFinaleMichani)
                {
                    lcd_update("Tlacitko 3", 1);
                    lcd_update("Osetri ", 0);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    lcd_update(osetrovanaPlodina, 0);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }

                char denAplikace[6];
                print_current_date(denAplikace, sizeof(denAplikace));
                for (int j = 0; j < pocetPostriku; j++)
                {
                    if (dataBazePostriku[j].id == idStruktury)
                    {
                        strcpy(dataBazePostriku[j].denPosledniAplikacePostriku, denAplikace);
                    }
                }
                ulozPostrikData();
                kvitujiFinaleMichani = false;
                zacaloMichani = false;
                xSemaphoreGive(Displej);
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
void mujTaskNaJadreJedna(void *pvParameter)
{
    nactiPostrikData();
    vTaskDelay(10 / portTICK_PERIOD_MS);

    while (1)
    {

        if (!mamNecoKmichani())
        {

            if (xSemaphoreTake(Displej, portMAX_DELAY))
            {
                // printf("\njsem v semaforu jadro 1\n");
                //  Example usage
                lcd_update(" CVUT FEL", 0);
                lcd_update(" PETR KUCERA", 1);
                xSemaphoreGive(Displej);
            }
        }
        for (int i = 0; i < pocetPostriku; i++)
        {

            if (mamNecoKmichani() && !zacaloMichani)
            {

                if (xSemaphoreTake(Displej, portMAX_DELAY))
                {
                    lcd_update("Chces michat?", 0);
                    lcd_update("Tlacitko 2", 1);
                    xSemaphoreGive(Displej);
                }
            }
            else
            {
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Displej = xSemaphoreCreateMutex();
    if (Displej == NULL)
    {
        printf("\nMutex nevytvoren\n");
    }
    Tlacitko1 = xSemaphoreCreateMutex();
    if (Tlacitko1 == NULL)
    {
        printf("\nMutex nevytvoren\n");
    }
    Tlacitko2 = xSemaphoreCreateMutex();
    if (Tlacitko2 == NULL)
    {
        printf("\nMutex nevytvoren\n");
    }
    Tlacitko3 = xSemaphoreCreateMutex();
    if (Tlacitko3 == NULL)
    {
        printf("\nMutex nevytvoren\n");
    }

    wifi_init_sta();
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();
    obtain_time();
    init_spiffs();
    start_webserver();
    lcd1602_init(RS, EN, D4, D5, D6, D7);
    gpio_set_direction(ledka, GPIO_MODE_OUTPUT);
    configure_interrupt1();
    configure_interrupt2();
    configure_interrupt3();

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
