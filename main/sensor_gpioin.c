#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "zigbee_init.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "esp_log.h"
#include "esp_err.h"
#include "cJSON.h"
#include "string.h"
#include "sensor.h"
#include "iot_button.h"
#include "sensor_gpioin.h"

static const char *TAG = "GPIO_IN";

static void reportAttribute(uint8_t endpoint, uint16_t clusterID, uint16_t attributeID, void *value, uint8_t value_length)
{
    esp_zb_zcl_report_attr_cmd_t cmd = {
        .zcl_basic_cmd = {
            .dst_addr_u.addr_short = 0x0000,
            .dst_endpoint = endpoint,
            .src_endpoint = endpoint,
        },
        .address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
        .clusterID = clusterID,
        .attributeID = attributeID,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    };
    esp_zb_zcl_attr_t *value_r = esp_zb_zcl_get_attribute(endpoint, clusterID, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, attributeID);
    memcpy(value_r->data_p, value, value_length);
    esp_zb_zcl_report_attr_cmd_req(&cmd);
}

void vTaskGPIOin(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    char *param_id = params->param_id;
    char id[30] = "";
    strcpy(id, param_id);
    int param_pin = params->param_pin;
    char *param_cluster = params->param_cluster;
    char cluster[30] = "";
    strcpy(cluster, param_cluster);
    int param_ep = params->param_ep;
    int param_int = params->param_int;
    // char *param_sensor_type = params->param_sensor_type;
    // char sensor_type[30] = "";
    // strcpy(sensor_type, param_sensor_type);

    // uint8_t button_state = gpio_get_level(param_pin);

    uint16_t last_state = gpio_get_level(param_pin);
    while (1)
    {
        uint16_t button_state = gpio_get_level(param_pin);
        if (button_state != last_state)
        {
            ESP_LOGI(TAG, "Button %d changed: %d", param_pin, button_state);
            last_state = button_state;
            if (strcmp(cluster, "BINARY") == 0)
            {
                reportAttribute(param_ep, ESP_ZB_ZCL_CLUSTER_ID_BINARY_INPUT, ESP_ZB_ZCL_ATTR_BINARY_INPUT_PRESENT_VALUE_ID, &button_state, 1);
            }
            else if (strcmp(cluster, "Contact") == 0)
            {
                esp_zb_zcl_ias_zone_status_change_notif_cmd_t cmd = {
                    .zcl_basic_cmd = {
                        .dst_addr_u.addr_short = 0x0000,
                        .dst_endpoint = param_ep,
                        .src_endpoint = param_ep,
                    },
                    .address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                    .zone_status = ESP_ZB_ZCL_IAS_ZONE_ZONE_STATUS_ALARM1 ? button_state : 0,
                    //    .zone_id = 0,
                    .delay = 0,
                };
                esp_zb_zcl_ias_zone_status_change_notif_cmd_req(&cmd);
            }
        }

        vTaskDelay(param_int / portTICK_PERIOD_MS);
    }
}

void gpioin(cJSON *sensor_json)
{

    cJSON *item = sensor_json->child;
    while (item != NULL)
    {

        cJSON *sensor_ = cJSON_GetObjectItemCaseSensitive(item, "sensor");
        //  cJSON *sensor_type_ = cJSON_GetObjectItemCaseSensitive(item, "sensor_type");
        cJSON *id_ = cJSON_GetObjectItemCaseSensitive(item, "id");
        cJSON *int_ = cJSON_GetObjectItemCaseSensitive(item, "int");
        cJSON *pin_ = cJSON_GetObjectItemCaseSensitive(item, "pin");
        cJSON *ep_ = cJSON_GetObjectItemCaseSensitive(item, "EP");
        cJSON *cluster_ = cJSON_GetObjectItemCaseSensitive(item, "claster");
        if (cJSON_IsString(sensor_) && cJSON_IsString(id_) && cJSON_IsNumber(pin_) && cJSON_IsNumber(int_) && cJSON_IsNumber(ep_) && cJSON_IsString(cluster_))
        {
            char *cluster = cluster_->valuestring;
            int EP = ep_->valueint;
            char *sensor = sensor_->valuestring;
            if (strcmp(cluster, "Contact") == 0 && strcmp(sensor, "GPIO_IN") == 0)
            {
                TaskParameters taskParams = {
                    .param_pin = pin_->valueint,
                    .param_ep = ep_->valueint,
                    .param_int = int_->valueint,
                    .param_cluster = cluster_->valuestring,
                    .param_id = id_->valuestring,
                    //   .param_sensor_type = sensor_type_->valuestring,
                };
                ESP_LOGW(TAG, "Task: %s created. Claster: %s EP: %d", sensor, cluster, EP);

                xTaskCreate(vTaskGPIOin, id_->valuestring, 4096, &taskParams, 5, NULL);
            }
            if (strcmp(cluster, "BINARY") == 0 && strcmp(sensor, "GPIO_IN") == 0)
            {
                TaskParameters taskParams = {
                    .param_pin = pin_->valueint,
                    .param_ep = ep_->valueint,
                    .param_int = int_->valueint,
                    .param_cluster = cluster_->valuestring,
                    .param_id = id_->valuestring,
                    //    .param_sensor_type = sensor_type_->valuestring,
                };
                ESP_LOGW(TAG, "Task: %s created. Claster: %s EP: %d", sensor, cluster, EP);
                xTaskCreate(vTaskGPIOin, id_->valuestring, 4096, &taskParams, 5, NULL);
            }
        }
        item = item->next;
    }
}
