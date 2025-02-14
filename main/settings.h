#ifndef MEEF_SETTINGS_H_
#define MEEF_SETTINGS_H_

#include "utils/common.h"
#include <cJSON.h>

typedef enum
{
    MATRIX_ROTATION_0 = 0,
    MATRIX_ROTATION_90,
    MATRIX_ROTATION_180,
    MATRIX_ROTATION_270
} matrix_rotation_t;

typedef struct
{
    struct
    {
        char devicename[16];
    } device;
    struct
    {
        uint32_t mode;
        bool wifi_present;
        bool wifi_enabled;
        bool wifi_conected;
        bool STA_conected;
        char STA_MAC[16];
        struct
        {
            bool dhcp;
            char ip[16];
            char staip[16];
            char netmask[16];
            char gateway[16];
            char dns[16];
        } ip;
        wifi_ap_config_t ap;
        wifi_sta_config_t sta;
    } wifi;
    struct
    {
        bool zigbee_present;
        bool zigbee_enabled;
        bool zigbee_conected;
        bool zigbee_dc_power;
        bool zigbee_router;
        bool zigbee_light_sleep;
        uint8_t lastBatteryPercentageRemaining;
        char modelname[16];
        char manufactuer[16];
        char manufactuer_id[16];

    } zigbee;
    struct
    {
        bool mqtt_enabled;
        bool mqtt_conected;
        char server[16];
        int port;
        char prefx[16];
        char user[16];
        char password[16];
        char path[16];

    } mqtt;
    struct
    {
        uint8_t block_width;
        uint8_t block_height;
        uint8_t h_blocks;
        uint8_t v_blocks;
        uint8_t type;
        uint8_t rotation;
        uint32_t current_limit;
    } leds;
} system_settings_t;

extern system_settings_t sys_settings;

typedef struct
{
    uint8_t fps;
    uint8_t brightness;
    size_t effect;
} volatile_settings_t;

extern volatile_settings_t vol_settings;

esp_err_t settings_init();

esp_err_t sys_settings_reset_nvs();

esp_err_t sys_settings_load_nvs();

esp_err_t sys_settings_save_nvs();

esp_err_t sys_settings_save_spiffs();

esp_err_t vol_settings_reset();

esp_err_t vol_settings_load();

esp_err_t vol_settings_save();

#endif /* MEEF_SETTINGS_H_ */
