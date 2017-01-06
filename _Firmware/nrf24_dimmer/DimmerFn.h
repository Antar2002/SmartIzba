#include <avr/eeprom.h>

#define TRIAC_IMPULS 20
#define TRIAC_STEPS_COUNT_DEFAULT 10

volatile int power_wave_length;
volatile int power_wave_length_cnt;
volatile uint8_t triac_steps_count;
volatile int triac_delay;					// Number of timer cycles after zero-cross when dimmer should give power to triac
volatile uint8_t min_power_perc; 			// Define lowest allowed level of power (important for fan)
volatile uint8_t start_triac_delay_perc;	// Define power level when user switch-on dimmer
volatile int start_triac_delay;
volatile int triac_cnt;
volatile uint8_t start_triac_impuls;		// Time (1/10 sec) after switching on when dimmer should send full impuls
volatile long start_triac_impuls_cnt;
volatile int triac_delay2;


void prepareStatus(uint8_t *sendBuf);
void incTriac();
void decTriac();
void setTriacFromBuf(uint8_t* buf);
void setTriac(int newDelay);
void setupTriac(uint8_t* buf);
void readParamsFromEmem();
void switchOff();


