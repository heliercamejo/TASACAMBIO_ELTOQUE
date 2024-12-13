#include "common.h"

uint16_t vdb = 56;
uint16_t vfw = 0;
uint16_t vhw = 0;

db_t db_config;

void app_main(void)
{
    spiffs_init();

    config_init();
}
