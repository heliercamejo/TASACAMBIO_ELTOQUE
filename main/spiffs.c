#include "common.h"
#include "string.h"

const char *TAG = "spiffs_t";
esp_vfs_spiffs_conf_t conf;

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

esp_err_t export_config(db_t *db, char *filename)
{
    db_device_t *device;
    
    FILE *fp = fopen(filename, "w");

    fprintf(fp, "v%hu;\n", vdb);

     // Write the contents of db_device
    fprintf(fp, "db_device:");
    device = db->device_config; // Get the first element of the linked list
    fprintf(fp,
    "!%s,!%s,!%s,%hhu,%lu,%lu,%lu,%lu,!%s,%hhu,%hhd;",
    device->mac, device->ssid, device->password, device->type,
    device->lan_ip, device->lan_gw, device->lan_dns,
    device->lan_sn,
    device->time, device->dst, device->timezone);
    fprintf(fp, "\n"); // End the line

    fprintf(fp, "\nEND_CONFIG"); // End the line
    ESP_LOGI(TAG, "EXPORT COMPLETE");

    // Close the file
    fclose(fp);
    return ESP_OK;
}

esp_err_t import_config(db_t *db, char *filename)
{
    char *buffer        = NULL;
    char *start_line    = NULL;
    char *end_line      = NULL;
    //char *component     = NULL;
    char *buffer_cpy    = NULL;

    uint32_t bufcpy_size = 0;
    int value = 0;

    uint32_t buffer_size = 0;

    if (spiffs_ready() == false)
    {
        return ESP_FAIL;
    }

    FILE *fp = fopen(filename, "r");
    // Check if the file is opened successfully
    if (fp == NULL)
    {
        //printf("Error: cannot open file %s\n", file_name);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,"file opened");
    fseek(fp, 0L, SEEK_END);
    buffer_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    ESP_LOGI(TAG,"file_size : %lu", buffer_size);

    buffer = malloc(buffer_size);

    fread(buffer, buffer_size, 1, fp);
    buffer[buffer_size] = '\0';
    fclose(fp);

    start_line = buffer;
    end_line = strchr(start_line, '\n') - 1; 
    
    bufcpy_size = end_line - start_line;
    buffer_cpy = malloc(bufcpy_size);
    memcpy(buffer_cpy,start_line,bufcpy_size);

    buffer_cpy[bufcpy_size] = '\0';
    sscanf(buffer_cpy,"v%d;",&value);
    free(buffer_cpy);
    
    ESP_LOGI(TAG,"file_version : %d", value);

    if(value != vdb)
    {
        //update_db();
    }
    return ESP_OK;
}