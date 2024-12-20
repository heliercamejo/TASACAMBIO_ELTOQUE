#include "common.h"

const char *TAG_db = "db_t";

esp_err_t config_init()
{
    ESP_LOGI(TAG_db, "config_ init");

    if(check_config_file())
    {
        ESP_LOGI(TAG_db, "import_config");
        import_config(&db_config);
    }
    else
    {
        ESP_LOGI(TAG_db, "default");
        db_device_t *device = malloc(sizeof(db_device_t));

        device->type = SOFT_AP;
        sprintf(device->ssid, "eltoque_api");
        sprintf(device->password, "abc123");
        
        get_mac(device->mac, device->type);
        device->lan_dns = 0;
        device->lan_gw = 0;
        device->lan_ip = 0;
        device->lan_sn = 0;
        sprintf(device->time, "time.windows.com");
        device->dst = 1;
        device->timezone = -5;
        
        db_config.device_config = device;

        export_config(&db_config);
    }
    
    return ESP_OK;
} 