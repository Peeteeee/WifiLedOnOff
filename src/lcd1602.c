
#include "lcd1602.h"

#define LOW 0
#define HIGH 1

uint16_t rs_pin, en_pin, d4_pin, d5_pin, d6_pin, d7_pin;

//STATIC FUNCTIONS

//Command write
static void lcd1602_cmdbits(bool d7_value, bool d6_value, bool d5_value, bool d4_value){
    //gpio_set_level(rs_pin, LOW);
    gpio_set_level(d7_pin, d7_value);
    gpio_set_level(d6_pin, d6_value);
    gpio_set_level(d5_pin, d5_value);
    gpio_set_level(d4_pin, d4_value);
    gpio_set_level(en_pin, HIGH);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(en_pin, LOW);
}

//Data write
static void lcd1602_writebits(bool d7_value, bool d6_value, bool d5_value, bool d4_value){
    gpio_set_level(rs_pin, HIGH);
    gpio_set_level(d7_pin, d7_value);
    gpio_set_level(d6_pin, d6_value);
    gpio_set_level(d5_pin, d5_value);
    gpio_set_level(d4_pin, d4_value);
    gpio_set_level(en_pin, HIGH);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(en_pin, LOW);
    gpio_set_level(rs_pin, LOW);
}

//Char write
static void lcd1602_char(char c){
    bool char_bits[8];
    int i;
    for(i = 0; i < 8; i++) char_bits[i] = (c >> i) & 1;
    lcd1602_writebits(char_bits[7], char_bits[6], char_bits[5], char_bits[4]);
    lcd1602_writebits(char_bits[3], char_bits[2], char_bits[1], char_bits[0]);
}

//LCD Initialization
esp_err_t lcd1602_init(uint16_t rs, uint16_t en,
                       uint16_t d4, uint16_t d5,
                       uint16_t d6, uint16_t d7)
{
    // Pin assignment
    rs_pin = rs;
    en_pin = en;
    d4_pin = d4;
    d5_pin = d5;
    d6_pin = d6;
    d7_pin = d7;

    // Pin config
    gpio_set_direction(rs, GPIO_MODE_OUTPUT);
    gpio_set_direction(en, GPIO_MODE_OUTPUT);
    gpio_set_direction(d4, GPIO_MODE_OUTPUT);
    gpio_set_direction(d5, GPIO_MODE_OUTPUT);
    gpio_set_direction(d6, GPIO_MODE_OUTPUT);
    gpio_set_direction(d7, GPIO_MODE_OUTPUT);

    // Reset rs and en pin values
    gpio_set_level(rs, LOW);
    gpio_set_level(en, LOW);

    //Boot up
    lcd1602_cmdbits(LOW, LOW, HIGH, HIGH);
    lcd1602_cmdbits(LOW, LOW, HIGH, HIGH);
    lcd1602_cmdbits(LOW, LOW, HIGH, HIGH);
    lcd1602_cmdbits(LOW, LOW, HIGH, LOW);

    //Function set - 4-Bits, 2 Rows, 5x7 char size
    lcd1602_cmdbits(LOW, LOW, HIGH, LOW);
    lcd1602_cmdbits(HIGH, LOW, LOW, LOW);

    //Display off
    lcd1602_cmdbits(LOW, LOW, LOW, LOW);
    lcd1602_cmdbits(HIGH, LOW, LOW, LOW);

    //Screen clear
    lcd1602_cmdbits(LOW, LOW, LOW, LOW);
    lcd1602_cmdbits(LOW, LOW, LOW, HIGH);

    //Entry mode set
    lcd1602_cmdbits(LOW, LOW, LOW, LOW);
    lcd1602_cmdbits(LOW, HIGH, HIGH, LOW);

    //Display On, Cursor Off, Blink Off
    lcd1602_cmdbits(LOW, LOW, LOW, LOW);
    lcd1602_cmdbits(HIGH, HIGH, LOW, LOW);

    //Return
    return ESP_OK;
}

//String write
esp_err_t lcd1602_write(char* string){
    int i;
    for(i = 0; i < strlen(string); i++){
        lcd1602_char(string[i]);
    }
    return ESP_OK;
}

//Screen clear
esp_err_t lcd1602_clear(){
    lcd1602_cmdbits(LOW, LOW, LOW, LOW);
    lcd1602_cmdbits(LOW, LOW, LOW, HIGH);
    return ESP_OK;
}

//Enable or disable cursor and blinking
esp_err_t lcd1602_set_behavior(bool cursor, bool blink){
    lcd1602_cmdbits(LOW, LOW, LOW, LOW);
    lcd1602_cmdbits(HIGH, HIGH, cursor, blink);
    return ESP_OK;
}

esp_err_t lcd1602_set_cursor(uint8_t line, uint8_t column){
    uint8_t addr = (line * 0x40) + column;
    bool addr_bits[7];
    int i;
    for(i = 0; i < 7; i++) addr_bits[i] = (addr >> i) & 1;
    lcd1602_cmdbits(HIGH, addr_bits[6], addr_bits[5], addr_bits[4]);
    lcd1602_cmdbits(addr_bits[3], addr_bits[2], addr_bits[1], addr_bits[0]);
    return ESP_OK;
}

esp_err_t lcd1602_shift_left(){
    lcd1602_cmdbits(LOW, LOW, LOW, HIGH);
    lcd1602_cmdbits(HIGH, LOW, LOW, LOW);
    return ESP_OK;
}

esp_err_t lcd1602_shift_right(){
    lcd1602_cmdbits(LOW, LOW, LOW, HIGH);
    lcd1602_cmdbits(HIGH, HIGH, LOW, LOW);
    return ESP_OK;
}
