#include <avr/eeprom.h>

void initAddress();
void sendAddressRequest();
void assignAddress(uint8_t *buffer);
char checkAddress(uint8_t *buffer);

//#define SRAM __attribute__((section(".noinit")))
