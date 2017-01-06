#include "nrf24/mirf.h"

//TIMER0
//#define SEC_TICKS 31250		//(8MHz - 32uS * 31250 = 1 sec)
//#define SEC_TICKS 46948		//(12MHz - 21.3uS * 46948 = 1 sec)
//TIMER1
//#define SEC_TICKS 122	//(8MHz)
#define SEC_TICKS 183	//(12MHz)

#define buffersize mirf_PAYLOAD
uint8_t buffer[buffersize];


// 0 - обычный режим
// 1 - запрос адреса
char mode;

// Адрес данного сенсора
// TODO: Сделать получение от хоста и хранение в EEPROM
extern char dev_address[2];
//u16 dev_address;
extern char noAddress;

void startLed(int cycles, long ledCnt);

