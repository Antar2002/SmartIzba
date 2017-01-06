#include <stdio.h>
#include "addressFn.h"
#include "nrf24/mirf.h"
#include "shared.h"

char dev_address[2];
EEMEM	uint8_t		dev_address_stored[2] = {0,0};

void initAddress()
{
	//dev_address[0] = 0x00;
	//dev_address[1] = 0x10;
	eeprom_read_block(dev_address, dev_address_stored, 2);
}

void sendAddressRequest()
{
	// ������������ ����� ��������
	uint8_t sendBuf[buffersize];
	// ����� ����������
	sendBuf[0] = dev_address[0];
	sendBuf[1] = dev_address[1];
	sendBuf[2] = 0;	// ������� �� ��������� ������
	sendBuf[3] = 2;	// ��� ����������

	// ��������� ������
	mirf_write(sendBuf);
}

// ���������� ������ ������ ����������
void assignAddress(uint8_t *buffer)
{
	dev_address[0] = buffer[3];
	dev_address[1] = buffer[4];
	eeprom_update_block(dev_address, dev_address_stored, 2);
	//noAddress = 0;
}

// ��������� ������� ��� ����� ���������� ��� ���� ����� 0 � ���-�� ��������� � ������ ��������� ������
char checkAddress(uint8_t *buffer)
{
	//int sum = 0;
	for(int i=0;i<2;i++)
	{
		//sum += dev_address[i];
		if((mode == 0 && buffer[i] != dev_address[i]) 
			//|| (mode == 1 && buffer[i] != 0)
			)
			return 0;
	}

	// ���� ����� ���������� �� ����� � ��� �� ��������� � ������ ��������� ������
	/*if(mode == 0 && sum == 0)
		return 0;*/

	return 1;
}
