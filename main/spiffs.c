#include "common.h"
#include "string.h"

const char *TAG = "spiffs_t";
esp_vfs_spiffs_conf_t conf;

bool check_config_file()
{
    if(!spiffs_ready())
    {
        ESP_LOGI(TAG, "spiffs doesn't init");
        return false;
    }

    struct stat st;
    if (stat("/spiffs/foo.txt", &st) < 0)
    {
        ESP_LOGI(TAG, "error open file");
        return false;
    }

    return true;
}

esp_err_t spiffs_init()
{
    conf.base_path = "/spiffs";
    conf.partition_label = NULL;
    conf.max_files = 5;
    conf.format_if_mount_failed = true;

        // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return ESP_FAIL;
        } else {
            ESP_LOGI(TAG, "SPIFFS_check() successful");
        }
    }
    return ESP_OK;
}

bool spiffs_ready()
{
    return esp_spiffs_mounted(conf.partition_label);
}

esp_err_t export_config(db_t *db)
{
    db_device_t *device;
    
    FILE *fp = fopen("/spiffs/device.cfg", "w");

    // Write the contents of device
    device = db->device_config; // Get the first element of the linked list
    fprintf(fp,
    "!%s,!%s,!%s,%hhu,%lu,%lu,%lu,%lu,!%s,%hhu,%hhd,",
    device->mac, device->ssid, device->password, device->type,
    device->lan_ip, device->lan_gw, device->lan_dns,
    device->lan_sn,
    device->time, device->dst, device->timezone);
    fprintf(fp, "\n"); // End the line

    // Close the file
    fclose(fp);
    return ESP_OK;
}

uint16_t file_size(FILE *fp)
{
    uint16_t file_size = 0;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}

esp_err_t import_config(db_t *db)
{
    char *buffer=NULL,*start_line=NULL, *end_line=NULL,*buffer_cpy=NULL;
    uint32_t bufcpy_size = 0, buffer_size = 0;
    uint32_t r  = 0;
    int64_t i;
    int value = 0;

    db_device_t *device;

    device = malloc(sizeof(db_device_t));

    if (spiffs_ready() == false)
        return ESP_FAIL;
    
    FILE *fp = fopen("/spiffs/device.cfg", "r");
    if (fp == NULL)
        return ESP_FAIL;

    buffer_size = file_size(fp);
    buffer = malloc(buffer_size);
    fread(buffer, buffer_size, 1, fp);
    buffer[buffer_size] = '\0';
    fclose(fp);

    start_line = buffer;
    end_line = strchr(start_line, '\n') - 1; 
    buffer_cpy = buffer;

    //device mac
    r = get_next_value(++buffer_cpy, &i, device->mac, ',');
    if (r)
    {
        buffer_cpy += r;
    }

    r = get_next_value(++buffer_cpy, &i, device->ssid, ',');
    if (r)
    {
        buffer_cpy += r;
    }

    r = get_next_value(++buffer_cpy, &i, device->password, ',');
    if (r)
    {
        buffer_cpy += r;
    }

    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->type = i;
        buffer_cpy += r;
    }
    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->lan_ip = i;
        buffer_cpy += r;
    }
    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->lan_gw = i;
        buffer_cpy += r;
    }
    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->lan_dns = i;
        buffer_cpy += r;
    }
    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->lan_sn = i;
        buffer_cpy += r;
    }
    
    r = get_next_value(++buffer_cpy, &i, device->time, ',');
    if (r)
    {
        buffer_cpy += r;
    }
    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->dst = i;
        buffer_cpy += r;
    }
    r = get_next_value(buffer_cpy, &i, NULL, ',');
    if (r)
    {
        device->timezone = i;
        buffer_cpy += r;
    }

    ESP_LOGI(TAG,"device\nmac:%s\nssid:%s\npassword:%s\ntype:%s\n",
    device->mac, device->ssid, device->password, (device->type == SOFT_AP)?"SOFT_AP": "STATIC");

    free(buffer);
    if(value != vdb)
    {
        //update_db();
    }
    return ESP_OK;
}

uint8_t get_next_value(char *s, long long *i, char *d, char delimiter)
{
    char *p;
    int length = 0;

    p = strchr(s, delimiter);
    if (p == NULL)
    {
        length = strlen(s);
        strncpy(d, s, length);
        d[length] = '\0';
    }
    else
    {
        length = p - s;
        if (d)
        {
            strncpy(d, s, length);
            d[length] = '\0';
        }
        else
        {
            *i = atoi(s);
        }
    }
    p = strchr(s, delimiter);
    if (p == NULL)
    {
        length = strlen(s);
        strncpy(d, s, length);
        d[length] = '\0';
    }
    else
    {
        length = p - s;
        if (d)
        {
            strncpy(d, s, length);
            d[length] = '\0';
        }
        else
        {
            *i = atoi(s);
        }
    }

    return length + 1;
}

esp_err_t remove_config()
{
    unlink("/spiffs/device.cfg");
    ESP_LOGI(TAG, "File remove");
    return ESP_OK;
}