#include "shared.h"
#define ZEROCROSS PD2
#define TRIAC PC4

#define LEDOUT PC0
#define BTN_PLUS PC1
#define BTN_MINUS PC2

#define NRF_IRQ PD3

#define BTN_PAUSE SEC_TICKS/2;

int addressRequestPause = 0;

char dataReceived = 0;
char sendInProgress = 0;

void init();
void initTimer();
void sendStatus();
