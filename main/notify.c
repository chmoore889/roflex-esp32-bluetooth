#include "notify.h"
#include "bleprph.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "host/ble_gatt.h"

#define NOTIFY_PERIOD_MS 250

static TimerHandle_t notifyTimerHandle;
static StaticTimer_t notifyTimer;
static uint16_t conn_handle;

static void doNotify(TimerHandle_t ev);

void initNotify(void) {
  notifyTimerHandle = xTimerCreateStatic("notifyTimer", pdMS_TO_TICKS(NOTIFY_PERIOD_MS), pdTRUE, (void *)0, doNotify, &notifyTimer);
}

void notifyStart(uint16_t notify_conn_handle) {
  conn_handle = notify_conn_handle;
  xTimerStart(notifyTimerHandle, 0);
}

void notifyStop(void) {
  xTimerStop(notifyTimerHandle, 0);
}

static void doNotify(xTimerHandle ev) {
  ble_gattc_notify(conn_handle, angleHandle);
}