#ifndef H_SENSOR
#define H_SENSOR

void initDevice(void);

bool getIsCounting(void);
uint8_t currRepCount(void);

void startCounting(uint16_t notify_conn_handle);
void stopCounting(void);
void shouldNotifyCount(bool);

void startReading(void);
void stopReading(void);

float lastAngle(void);

#endif