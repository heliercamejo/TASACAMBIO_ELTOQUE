#include "common.h"
#include "string.h"

const char *TAG_wifi = "wifi_t";

void get_mac(char *mac_address, wifi_ap_e type)
{
    uint8_t *mac = malloc(6);
    switch (type)
    {
    case SOFT_AP:
        esp_read_mac(mac,ESP_MAC_WIFI_STA);
        break;
    case STATIC:
        esp_read_mac(mac,ESP_MAC_WIFI_SOFTAP);
        break;
    default:
        break;
    }
    sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    ESP_LOGI(TAG_wifi,"mac: %s", mac_address);
    free(mac);
}

esp_err_t wifi_init()
{
    switch (db_config.device_config->type)
    {
    case SOFT_AP:
        /* code */
        break;

    case STATIC:
        /* code */
        break;
    
    default:
        break;
    }

    return ESP_OK;
}