#include <avr/interrupt.h>
#include "DimmerFn.h"
#include "shared.h"

uint8_t		EEMEM	triac_steps_count_stored = 10;
uint8_t		EEMEM	start_triac_delay_perc_stored = 30;
uint8_t		EEMEM	min_power_perc_stored = 20; 		//400;
uint8_t		EEMEM	start_triac_impuls_stored = 20;

void prepareStatus(uint8_t *sendBuf){

	// Wave length
	sendBuf[3] = (power_wave_length>>8);
	sendBuf[4] = power_wave_length;

	// Curent delay
	sendBuf[5] = (triac_delay>>8);
	sendBuf[6] = triac_delay;

	// Steps count
	sendBuf[7] = triac_steps_count;

	// Triac impuls
	sendBuf[8] = TRIAC_IMPULS;

	sendBuf[9] = min_power_perc;

	sendBuf[10] = start_triac_delay_perc;

	sendBuf[11] = start_triac_impuls;

	return sendBuf;
}


void incTriac()
{

	// Не давать регулировать, пока идет начальный импульс
	if(triac_delay2>0) return;

	int step = power_wave_length / triac_steps_count;	// 30

	// Если был выключен, стартовать с заданной мощности
	if(triac_delay >= power_wave_length){
		switchOn();
	}
	else if(triac_delay > step + TRIAC_IMPULS + TRIAC_IMPULS){
		triac_delay -= step;
		startLed(1, SEC_TICKS/8);
	}
	else{
		triac_delay = TRIAC_IMPULS;
		startLed(3, SEC_TICKS/8);
	}
}


void decTriac()
{

	// Не давать регулировать, пока идет начальный импульс
	if(triac_delay2>0) return;

	int step = power_wave_length / triac_steps_count;	// 30
	int max_triac_delay = power_wave_length - (power_wave_length * min_power_perc / 100);
	if((triac_delay < power_wave_length - TRIAC_IMPULS - step && max_triac_delay <= 0) ||
		(max_triac_delay > 0 && triac_delay < max_triac_delay - step)){
		triac_delay += step;
		startLed(1, SEC_TICKS/8);
	}
	else{
		// Выключить совсем, если новое значение оставляет слишком низкую мощность
		switchOff();
		startLed(3, SEC_TICKS/8);
	}
}


void setTriacFromBuf(uint8_t* buf)
{

	int newDelay;

	newDelay = (buf[0]<<8);
	newDelay += buf[1];

	setTriac(newDelay);
}


void setTriac(int newDelay)
{

	// Не давать регулировать, пока идет начальный импульс
	if(triac_delay2>0) return;

	if(triac_delay > power_wave_length)
		switchOn();
	else if(newDelay<0)
		triac_delay = 0;
	else if(newDelay > power_wave_length - TRIAC_IMPULS)
		switchOff();
	else
		triac_delay = newDelay;
}


void setupTriac(uint8_t* buf)
{

	triac_steps_count = buf[0];

	min_power_perc = buf[1];

	start_triac_delay_perc = buf[2];

	start_triac_impuls = buf[3];
	
	eeprom_write_byte(&triac_steps_count_stored, triac_steps_count);
	eeprom_write_byte(&start_triac_delay_perc_stored, start_triac_delay_perc);

	//max_triac_delay_buf[0] = max_triac_delay>>8;
	//max_triac_delay_buf[1] = max_triac_delay;
	eeprom_write_byte(&min_power_perc_stored, min_power_perc);

	eeprom_write_byte(&start_triac_impuls_stored, start_triac_impuls);
}


void readParamsFromEmem(){

	uint8_t max_triac_delay_buf[2];

	// Кол-во шагов регулирования диммера
	triac_steps_count = eeprom_read_byte(&triac_steps_count_stored);

	// Мощность, на которой включается диммер после полного выключения
	start_triac_delay_perc = eeprom_read_byte(&start_triac_delay_perc_stored);

	// Минимальная яркость (мощность) ниже которой только полное вылкючение
	min_power_perc = eeprom_read_byte(&min_power_perc_stored);
	//max_triac_delay = (max_triac_delay_buf[0]<<8);
	//max_triac_delay += max_triac_delay_buf[1];
	/*triac_steps_count = 10;
	start_triac_delay_perc = 50;
	max_triac_delay = 234;*/

	start_triac_impuls = eeprom_read_byte(&start_triac_impuls_stored);

}


void switchOff(){
	triac_delay = power_wave_length * 2;
}


void switchOn(){
	if(start_triac_impuls > 0){
		cli();
		triac_delay2 = getStartDelay();
		//start_triac_impuls_cnt = 4695 * start_triac_impuls;
		start_triac_impuls_cnt = SEC_TICKS * start_triac_impuls / 10;
		triac_delay = TRIAC_IMPULS;
		startLed(2, SEC_TICKS/8);
		sei();
	}
	else{
		cli();
		triac_delay2 = 0;
		triac_delay = getStartDelay();
		sei();
	}
}


int getStartDelay(){
	return power_wave_length - (power_wave_length * start_triac_delay_perc / 100);
}
