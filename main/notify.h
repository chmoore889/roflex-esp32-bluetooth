#ifndef H_NOTIFY
#define H_NOTIFY

#include <stdint.h>

void notifyStart(uint16_t notify_conn_handle);
void notifyStop(void);

void initNotify(void);

#endif