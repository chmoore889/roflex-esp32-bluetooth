#include <math.h>
#include "bleprph.h"
#include "sensor.h"
#include "i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "host/ble_gatt.h"

#define SQUARE(x) (x*x)
#define RADIANS_TO_DEGREES(x) (x*57.2957795)
#define READ_PERIOD_MS 100

static const char *tag = "Sensor";

enum RepState{Low, High, Travel} countState = Low, lastState = Low;

static void readAngle();
static void readDeviceTsk(TimerHandle_t xTimer);
static void processForCounter(float angle);

static void notifyRepCount();

static int16_t bnoaccX = 0, bnoaccY = 0, bnoaccZ = 0;
float lastAngleCalc = 0.0f;
static StaticTimer_t readDeviceTskTimer;
static TimerHandle_t readDeviceTskTimerHandle;

static uint16_t conn_handle;

static uint8_t repCount = 0;
static bool isCounting = false;
static bool shouldNotify = false;

static inline uint16_t arrTo16Bit(uint8_t* arr) {
  return (arr[1]<<8)|arr[0];
}

static void processForCounter(float angle) {
  if (isCounting){
    if(countState == Low) {
      if(angle > -5 && angle < 30) {
        lastState = countState;
        countState = Travel;
      }
    } else if(countState == Travel) {
      if(angle <= -5) {
        if(lastState == High) {
          repCount++;
          notifyRepCount();
        }

        lastState = countState;
        countState = Low;
      } else if (angle >= 30) {
        lastState = countState;
        countState = High;
      }
    } else if(countState == High) {
      if(angle > -5 && angle < 30) {
        lastState = countState;
        countState = Travel;
      }
    }

    ESP_LOGI(tag, "Counting: %d - State: %u - Angle: %f", repCount, countState, angle);
  }
}

void startCounting(uint16_t notify_conn_handle) {
  isCounting = true;
  conn_handle = notify_conn_handle;
  
  notifyRepCount();
  startReading();
}

void stopCounting(void) {
  isCounting = false;

  notifyRepCount();
  stopReading();
}

uint8_t currRepCount(void) {
  return repCount;
}

bool getIsCounting(void) {
  return isCounting;
}

void overrideCount(uint8_t count) {
  repCount = count;
  notifyRepCount();
}

float lastAngle(void) {
  if(xTimerIsTimerActive(readDeviceTskTimerHandle) == pdFALSE) {
    readAngle();
  }

  return lastAngleCalc;
}

void initDevice() {
  uint8_t data;
  i2cInit();

  //Ensure BNO is correct
  ESP_ERROR_CHECK(readDevice(BNO, 0x00, (uint8_t*) &data, sizeof(data)));
  if(data != 0xA0) {
      ESP_LOGE(tag, "BAD BNO DEVICE");
      exit(1);
  }
  //Check self-test on BNO
  ESP_ERROR_CHECK(readDevice(BNO, 0x36, (uint8_t*) &data, sizeof(data)));
  data = 0xF & data;
  if(data != 0xF) {
      ESP_LOGE(tag, "BNO failed self-test: 0x%X", data);
      exit(1);
  }
  //Set OP mode of BNO
  data = 0xC;
  ESP_ERROR_CHECK(writeDevice(BNO, 0x3D, data));

  readDeviceTskTimerHandle = xTimerCreateStatic("readDevice", pdMS_TO_TICKS(READ_PERIOD_MS), true, NULL, readDeviceTsk, &readDeviceTskTimer);
  if (readDeviceTskTimerHandle == NULL) {
    ESP_LOGE(tag, "Failed to create timer");
    exit(1);
  }
}

static void readAngle() {
  uint8_t data[sizeof(bnoaccX) + sizeof(bnoaccY) + sizeof(bnoaccZ)];
  ESP_ERROR_CHECK(readDevice(BNO, 0x2E, data, sizeof(data)));

  bnoaccX = arrTo16Bit(data);
  bnoaccY = arrTo16Bit(data + sizeof(bnoaccX));
  bnoaccZ = arrTo16Bit(data + sizeof(bnoaccX) + sizeof(bnoaccY));
  lastAngleCalc = RADIANS_TO_DEGREES(asinf(bnoaccX / sqrtf((float) (SQUARE(bnoaccX) + SQUARE(bnoaccY) + SQUARE(bnoaccZ)))));
  ESP_LOGI(tag, "BNO Grav X: %d\tY: %d\tZ: %d - Angle %f\n", bnoaccX, bnoaccY, bnoaccZ, lastAngleCalc);
}

static void readDeviceTsk(TimerHandle_t xTimer) {
  readAngle();
  processForCounter(lastAngle());
}

void startReading() {
  BaseType_t ret = xTimerStart(readDeviceTskTimerHandle, 0);
  if (ret != pdPASS) {
    ESP_LOGE(tag, "Failed to start timer");
    exit(1);
  }
}

void stopReading() {
  BaseType_t ret = xTimerStop(readDeviceTskTimerHandle, 0);
  if (ret != pdPASS) {
    ESP_LOGE(tag, "Failed to stop timer");
    exit(1);
  }
}

static void notifyRepCount() {
  if(shouldNotify) {
    ble_gattc_indicate(conn_handle, repHandle);
  }
}

void shouldNotifyCount(bool should) {
  shouldNotify = should;
}