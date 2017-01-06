#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nrf24/mirf.h"
#include "nrf24/nRF24L01.h"
#include "addressFn.h"
#include "shared.h"
#include "DimmerNrf.h"
#include "shared.h"
#include "DimmerFn.h"

volatile long led_cnt_max;
volatile long led_cnt;
volatile int led_cycle;

volatile int btn_pause_cnt;

uint8_t rxaddr[5] = {0x01, 0x02, 0x03, 0x04, 0x05};

ISR(TIMER0_OVF_vect)  // Обработчик прерываний
{
	cli();

	if(triac_cnt>0){
		triac_cnt--;
		if(triac_cnt==TRIAC_IMPULS){
			// Switch triac ON
			PORTC &= ~(1<<TRIAC);
		}
	}
	else
	{
		// Switch triac OFF
		PORTC |= (1<<TRIAC);
	}

	power_wave_length_cnt++;
	sei();
}

ISR(TIMER1_OVF_vect){

	cli();

	if(start_triac_impuls_cnt > 0)
		start_triac_impuls_cnt--;
	if(triac_delay2 > 0 && start_triac_impuls_cnt <= 0){
		triac_delay = triac_delay2;
		triac_delay2 = 0;
		startLed(2, SEC_TICKS/8);
	}

	if(led_cnt<=0){
		if(led_cycle<=0){
			PORTC &= ~(1<<LEDOUT);
		}
		else{
			PORTC ^= (1<<LEDOUT);
			led_cycle--;
			led_cnt = led_cnt_max;
		}
	}
	else{
		//PORTC |= (1<<LEDOUT);
		led_cnt--;
	}

	if(btn_pause_cnt>0){
		btn_pause_cnt--;
	}

	if(addressRequestPause>0){
		addressRequestPause--;
	}
	sei();
}

ISR(INT0_vect)	// Interupt from zero-cross detector (50Hz * 2 - each 10ms - 469 timer interupts)
{
	//btnTimer = 1;
	power_wave_length = power_wave_length_cnt;

	// If triac_delay is not initialized
	/*if(triac_delay<0) 
	{
		triac_delay = power_wave_length * 2;
		sendStatus();
	}*/

	triac_cnt = triac_delay + TRIAC_IMPULS;
	power_wave_length_cnt = 0;
}

ISR(INT1_vect)	// Interupt from NRF
{
	cli();
startLed(1, SEC_TICKS);
	uint8_t status = mirf_get_status();
	if(status & (1<<RX_DR)){
		//mirf_write_register(STATUS, (1<<RX_DR));
		//mirf_flush_rx();
		if(dataReceived == 0){
			mirf_read(buffer);
			//dataReceived = 1;
		}
		//mirf_write_register(STATUS, (1<<RX_DR));
		mirf_reset();
		mirf_flush_rx();
		sendStatus();
	}
	else if((status & (1<<TX_DS)) || (status & (1<<MAX_RT))){
		afterWriteData(status);
		sendInProgress = 0;
	}
	sei();
}

void init()
{

	// Init Zero-Cross pin
	DDRD &= ~((1<<ZEROCROSS) | (1<<NRF_IRQ));
	PORTD |= (1<<ZEROCROSS) | (1<<NRF_IRQ);

	// Init Triac pin
	DDRC |= (1<<TRIAC);
	PORTC |= (1<<TRIAC);

	// Init buttons pins
	DDRC &= ~(1<<BTN_PLUS) & ~(1<<BTN_MINUS);
	PORTC |= (1<<BTN_PLUS) | (1<<BTN_MINUS);

	// Init LED
	DDRC |= (1<<LEDOUT);
	PORTC &= ~(1<<LEDOUT);

	// Init INT0 (Zero cross)
	//GIMSK = (1<<INT0);
	GICR = (1<<INT0) | (1<<INT1);
	MCUCR |= (1<<ISC01) | (1<<ISC00) | (1<<ISC11) | (0<<ISC10);

	// Init NRF
	mirf_init();
	_delay_ms(50);

	sei();

	mirf_config();

	mirf_set_rxaddr(0, rxaddr);
	mirf_set_txaddr(rxaddr);

	initTimer();

}

// Init timer
void initTimer()
{
	TIMSK |= (1<<TOIE0) | (1<<TOIE1);	// Включить прерывание по переполнению таймера
	//TCCR0 |= (1<<CS02) | (0<<CS01) | (0<<CS00);		// xtall/256		21.3 uS for 12000000 overflow 5.461ms (182.12 in sec)
	//TCCR0 |= (0<<CS02) | (1<<CS01) | (0<<CS00);		// xtall/8			5.3 uS for  12000000 overflow 1.365ms ( in sec)
	TCCR0 |= (1<<CS00);									// xtall			0.67 uS for 12000000 overflow 170.66us (46875 in sec)
	TCNT0 = 0x00;

	TCCR1B |= (1<<CS10);		// xtall (12MHz)	0.083us * 65536 = 5.4613 mS (183 in sec)
								// xtall (8MHz)		0.125us * 65536 = 8.192 mS (122 in sec)
	TCNT1 = 0x00;
}


int main(void)
{

	power_wave_length = 234;
	triac_delay = 500;
	start_triac_impuls_cnt = 0;

	init();

	if((~PINC & (1<<BTN_PLUS)) | (~PINC & (1<<BTN_MINUS))){

		mode = 1;
		dev_address[0] = rand();
		dev_address[1] = rand();

		// Запросить адрес
		sendAddressRequest();
		addressRequestPause = SEC_TICKS;
		startLed(5, SEC_TICKS/6);

	}
	else{
		mode = 0;

		initAddress();

		readParamsFromEmem();

		//startLed(1, 23437);
		startLed(1, SEC_TICKS);
	}

	while(1) 
	{

		if(power_wave_length > 0 && btn_pause_cnt==0){
			if(~PINC & (1<<BTN_PLUS)){
				incTriac();
				sendStatus();
				btn_pause_cnt = BTN_PAUSE;
			}
			if(~PINC & (1<<BTN_MINUS)){
				decTriac();
				sendStatus();
				btn_pause_cnt = BTN_PAUSE;
			}
		}

		// Пришли данные
		//char rx_fifo = check_rx_fifo();

		if(dataReceived == 1)
		{
			//mirf_read(buffer);
			// Проверить, что это команда для данного устройства
			if(checkAddress(buffer))
			{
				// Assign address
				if(mode==1)
				{
					if(buffer[2] == 0){		// Получение адреса уст-ва
						assignAddress(buffer);
						mode = 0;
					}
				}
				else{
					// Process command
					switch(buffer[2]){
						case 1:
							sendStatus();
							startLed(1, SEC_TICKS/6);
							break;
						case 2:
							incTriac();
							sendStatus();
							break;
						case 3:
							decTriac();
							sendStatus();
							break;
						case 4:
							setTriacFromBuf(buffer[3]);
							sendStatus();
							startLed(1, SEC_TICKS/6);
							break;
						case 5:
							setupTriac(buffer[3]);
							sendStatus();
							break;
					}
				}
			}
			//mirf_flush_rx();
			dataReceived = 0;
		}

		// Перезапросить адрес
		if(mode==1 && addressRequestPause==0){
			sendAddressRequest();
			addressRequestPause = SEC_TICKS;
			startLed(5, SEC_TICKS/6);
		}
	}
}


void startLed(int cycles, long ledCnt)
{
	PORTC |= (1<<LEDOUT);
	led_cycle = (cycles * 2) - 1;
	led_cnt_max = ledCnt;
	led_cnt = led_cnt_max;
}


void sendStatus(){

	if(sendInProgress == 1)
		return;

	sendInProgress = 1;

	uint8_t sendBuf[mirf_PAYLOAD];

	prepareStatus(sendBuf);

	// Адрес устройства
	sendBuf[0] = dev_address[0];
	sendBuf[1] = dev_address[1];
	sendBuf[2] = 1;	// Код операции

	mirf_write(sendBuf);

}

