#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_mac.h"

extern uint16_t vdb;
extern uint16_t vfw;
extern uint16_t vhw;

typedef enum wifi_ap_e
{
    SOFT_AP = 0,
    STATIC 
}wifi_ap_e;

typedef struct db_device_t
{
    char mac[20];
    char ssid[30];
    char password[30];
    wifi_ap_e type;
    char time[20];
    uint8_t dst;
    int8_t timezone;
    uint32_t lan_ip;
    uint32_t lan_gw;
    uint32_t lan_dns;
    uint32_t lan_sn;
}db_device_t;


typedef struct db_t
{
    /* data */
    db_device_t *device_config;
}db_t;

extern db_t db_config;

esp_err_t spiffs_init();
bool spiffs_ready();
esp_err_t export_config(db_t *db);
esp_err_t import_config(db_t *db);
bool check_config_file();
esp_err_t remove_config();
uint8_t get_next_value(char *s, long long *i, char *d, char delimiter);

esp_err_t wifi_init();
void get_mac(char *mac_address, wifi_ap_e type);

esp_err_t config_init();


#endif //_COMMON_H_