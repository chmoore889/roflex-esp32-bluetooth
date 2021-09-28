#include "driver/i2c.h"
#include "i2c.h"

//static const char* TAG = "TestModule";

esp_err_t writeDevice(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
  i2c_cmd_handle_t link = i2c_cmd_link_create();

  i2c_master_start(link);
  i2c_master_write_byte(link, dev_addr << 1 | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(link, reg_addr, true);
  i2c_master_write_byte(link, data, true);
  i2c_master_stop(link);

  esp_err_t res = i2c_master_cmd_begin(I2C_MASTER_NUM, link, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

  i2c_cmd_link_delete(link);
  return res;
}

esp_err_t readDevice(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, size_t dataSize) {
  i2c_cmd_handle_t link = i2c_cmd_link_create();

  i2c_master_start(link);
  i2c_master_write_byte(link, dev_addr << 1 | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(link, reg_addr, true);

  i2c_master_start(link);
  i2c_master_write_byte(link, dev_addr << 1 | I2C_MASTER_READ, true);
  for(size_t x = 0; x < dataSize; x++) {
    i2c_master_read_byte(link, data + x, x != (dataSize-1) ? I2C_MASTER_ACK : I2C_MASTER_NACK);
  }
  i2c_master_stop(link);

  esp_err_t res = i2c_master_cmd_begin(I2C_MASTER_NUM, link, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

  i2c_cmd_link_delete(link);
  return res;
}

void i2cInit() {
  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

  int timeout;
  i2c_get_timeout(I2C_MASTER_NUM, &timeout);
  timeout *= 5;
  i2c_set_timeout(I2C_MASTER_NUM, timeout);
}