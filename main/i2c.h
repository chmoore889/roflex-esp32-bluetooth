#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           23
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       10000

#define LSM9DS1_M   0x1E
#define LSM9DS1_AG  0x6B
#define BNO  0x28

void i2cInit(void);
esp_err_t writeDevice(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
esp_err_t readDevice(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, size_t dataSize);