#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ssp/mmp_spi.h"

#define MAXLEN 300
#define MINLEN 1

void* TestFunc(void* arg)
{
	int i, j;
    char getstr[256];
    char sendtr[256];
	int len = 0;
	int32_t result = -1;
	uint32_t datasize;
	uint32_t size;
	uint8_t* txbuf;
	uint8_t* rxbuf;

	
	uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
					  0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,  
					  0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
					  0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
					  0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
					  0x32, 0x33, 0x44, 0x45, 0x56, 0x7E, 0x8C, 0xFE, 0x5A, 0xCC,};


    //itpRegisterDevice(ITP_DEVICE_UART1, &itpDeviceUart1);
    //ioctl(ITP_DEVICE_UART1, ITP_IOCTL_INIT, NULL);
    //ioctl(ITP_DEVICE_UART1, ITP_IOCTL_RESET, CFG_UART1_BAUDRATE);

    printf("Start spi test hello!\n");



    result = mmpSpiInitialize(0,0, CPO_0_CPH_0, SPI_CLK_20M);
    if (result)
    	printf("--- SPI_0 init Error ---\n");

   	//result = mmpSpiInitialize(1); 	
    //if (result)
    //	printf("--- SPI_1 init Error ---\n");
    
    datasize = sizeof(data)/sizeof(uint8_t);
    while(1)
    {	
		size = (rand()%(MAXLEN-MINLEN+1))+MINLEN;
		
		size = (size >> 2) << 2;
		printf("size = %d\n", size);
		
		txbuf =(uint8_t*)calloc(size, sizeof(uint8_t));
		if (!txbuf)
			printf("alloc tx buf error !!!\n");
		
		rxbuf =(uint8_t*)calloc(size, sizeof(uint8_t));
		if (!rxbuf)
			printf("alloc rx buf error !!!\n");
		
		for (i=0; i<size; i++)
		{
			*(txbuf+i) = data[rand()% datasize];
			//printf("txbuf[%d] = 0x%x\n", i , *(txbuf+i));
		}

		//PalMemcpy(rxbuf, txbuf, size);
    
		mmpSpiTransfer(0, txbuf, rxbuf, size);

		//for (i=0; i<size; i++)
		//{
		//	printf("rxbuf[%d] = 0x%x\n", i, *(rxbuf+i));
		//}
		
		if (memcmp(txbuf, rxbuf, size) == 0)
			printf(" SPI SUCCESS\n");
		else
			printf(" SPI ERROR\n");

		
		free(txbuf);		
		free(rxbuf);
		usleep(2000000);
	}
	
}

void* TestFunc1(void* arg)
{
	printf("spi test function1\n");
	int fd;
	int32_t result = -1;
	uint32_t timeOut = 0x100000;
	ITPSpiInfo SpiInfo = {0};
	uint8_t	command[5] = {0};
	uint8_t id1[2]     = {0};
	uint8_t id2[3]     = {0};
	uint8_t data[2]    = {0};
	uint8_t value      = 1;
	
	itpRegisterDevice(ITP_DEVICE_SPI, &itpDeviceSpi0);
    ioctl(ITP_DEVICE_SPI, ITP_IOCTL_INIT, NULL);

	fd = open(":spi:0", O_RDONLY);
	if (!fd)
		printf("--- open device spi0 fail ---\n");
	else
		printf("fd = %d\n", fd);

	command[0] = 0x9F;
	
	SpiInfo.readWriteFunc = ITP_SPI_PIO_READ;	
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &id2;
	SpiInfo.dataBufferSize = 3;
	read(fd, &SpiInfo, 1);
	printf("spiinfo 0x%x, 0x%x, 0x%x\n", id2[0],id2[1], id2[2]);

	SpiInfo.readWriteFunc = ITP_SPI_PIO_READ;	
	memset(command, 0, 5);
	memset(data, 0, 2);
	command[0]= 0x05;
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &value;
	SpiInfo.dataBufferSize = 1;
	_read(fd, &SpiInfo, 1);
	printf("1. value = 0x%x", value);
	if ((value & 0x1C) != 0)
		printf("-- nor in write protect mode --\n");
	else
		printf("-- nor write unlock success --\n");

	//unlock nor write procedure
	//write enable
	SpiInfo.readWriteFunc = ITP_SPI_PIO_WRITE;
	memset(command, 0, 5);
	command[0]= 0x06;
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &data;
	SpiInfo.dataBufferSize = 2;
	result = _write(fd, &SpiInfo, 1);
	//Write Status
	SpiInfo.readWriteFunc = ITP_SPI_PIO_WRITE;
	memset(command, 0, 5);
	command[0]= 0x07;
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &data;
	SpiInfo.dataBufferSize = 2;
	result = _write(fd, &SpiInfo, 1);
	//wait nor ready
	SpiInfo.readWriteFunc = ITP_SPI_PIO_READ;	
	memset(command, 0, 5);
	memset(data, 0, 2);
	command[0]= 0x05;
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &value;
	SpiInfo.dataBufferSize = 1;
	while(1)
	{
		_read(fd, &SpiInfo, 1);
		if (value & 0x1)
		{
			if(timeOut--)
			{
				continue;
			}
			else
			{
				printf("-- device is busy --\n");
				break;
			}
		}
		else
		{
			printf("-- device Ready --\n");
			break;
		}
	}
	//read nor write protect status
	SpiInfo.readWriteFunc = ITP_SPI_PIO_READ;	
	memset(command, 0, 5);
	memset(data, 0, 2);
	command[0]= 0x05;
	SpiInfo.cmdBuffer = &command;
	SpiInfo.cmdBufferSize = 1;
	SpiInfo.dataBuffer = &value;
	SpiInfo.dataBufferSize = 1;
	_read(fd, &SpiInfo, 1);
	printf("2. value = 0x%x", value);
	if ((value & 0x1C) != 0)
		printf("-- nor in write protect mode --\n");
	else
		printf("-- nor write unlock success --\n");
}

