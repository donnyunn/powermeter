#include "i2c_master.h"

#include "driver/i2c.h"

#define I2C_MASTER_NUM CONFIG_I2C_MASTER_PORT_NUM
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL I2C_MASTER_ACK                  /*!< I2C ack value */
#define NACK_VAL I2C_MASTER_NACK                /*!< I2C nack value */
#define LAST_NACK_VAL I2C_MASTER_LAST_NACK      /*!< I2C last nack value */

static const char *TAG = "Dulab I2C";
static i2c_master_t i2c_master = {
    NULL,
    NULL,
};

static esp_err_t i2c_read(uint8_t addr_7bit, uint8_t * data, int len)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_LOGI(TAG, "[APP] %s", __func__);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr_7bit << 1 | READ_BIT, ACK_CHECK_EN);
    for (int i = 0; i < len; i++) {
        i2c_master_read_byte(cmd, data + i, (i != (len-1)) ? ACK_VAL : LAST_NACK_VAL);
    }
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t i2c_write(uint8_t addr_7bit, uint8_t * data, int len)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_LOGI(TAG, "[APP] %s", __func__);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr_7bit << 1 | WRITE_BIT, ACK_CHECK_EN);
    for (int i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, data[i], ACK_CHECK_EN);
    }
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

i2c_master_t * i2c_init(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "[APP] i2c param config error");
    }
    err = i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "[APP] i2c driver install error");
    }

    i2c_master.read = i2c_read;
    i2c_master.write = i2c_write;

    return &i2c_master;
}