#include "common.h"

extern db_t db_config;

esp_err_t config_init()
{
    db_device_t device;

    device.type = SOFT_AP;
    sprintf(device.ssid, "eltoque_api");
    sprintf(device.password, "abc123");
    
    device.mac[0] = 0;
    device.lan_dns = 0;
    device.lan_gw = 0;
    device.lan_ip = 0;
    device.lan_sn = 0;
    sprintf(device.time, "time.windows.com");
    device.dst = 1;
    device.timezone = -5;
    
    db_config.device_config = &device;

    export_config(&db_config,"/spiffs/sw.cfg");

    import_config(&db_config,"/spiffs/sw.cfg");

    return ESP_OK;
} 