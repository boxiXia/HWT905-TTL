
#include "Com.h"
#include <windows.h>
#include <stdio.h>

#define		TOTAL_PORT_NUM		65
#define		START_PORT_NUM		0

#define		iBufferSize 250
#define     UARTBufferLength 2000

static HANDLE		 hComDev[TOTAL_PORT_NUM]         ={NULL};
static unsigned long long ulComMask = 0;
static HANDLE		 hCOMThread[TOTAL_PORT_NUM]      ={NULL};
static OVERLAPPED	 stcWriteStatus[TOTAL_PORT_NUM]  = {0};
static OVERLAPPED	 stcReadStatus[TOTAL_PORT_NUM]   = {0};


static volatile char chrUARTBuffers[TOTAL_PORT_NUM][UARTBufferLength]={0};
static volatile unsigned long ulUARTBufferStart[TOTAL_PORT_NUM]={0}, ulUARTBufferEnd[UARTBufferLength]={0};

unsigned short CollectUARTData(const unsigned long ulCOMNo,char chrUARTBufferOutput[])
{
	unsigned long ulLength=0;
	unsigned long ulEnd ;
	unsigned long ulStart ;

	ulEnd = ulUARTBufferEnd[ulCOMNo];
	ulStart = ulUARTBufferStart[ulCOMNo];
	if (ulEnd == ulStart)
		return(0);
	if (ulEnd > ulStart)
	{
		memcpy((void*)chrUARTBufferOutput,(void*)(chrUARTBuffers[ulCOMNo]+ulStart),ulEnd-ulStart);
		ulLength = ulEnd-ulStart;
	}
	else
	{
		memcpy((void*)chrUARTBufferOutput,(void*)(chrUARTBuffers[ulCOMNo]+ulStart),UARTBufferLength-ulStart);
		if ( ulEnd != 0 )
		{
			memcpy((void*)(chrUARTBufferOutput+(UARTBufferLength-ulStart)),(void*)chrUARTBuffers[ulCOMNo],ulEnd);
		}
		ulLength = UARTBufferLength+ulEnd-ulStart;
	}
	ulUARTBufferStart[ulCOMNo] = ulEnd;
	return (unsigned short) ulLength;
}

signed char SendUARTMessageLength(const unsigned long ulChannelNo, const char chrSendBuffer[],const unsigned short usLen)
{
	DWORD iR;
	DWORD dwRes;
	DCB dcb;
	char chrDataToSend[1000] = {0};
	memcpy(chrDataToSend,chrSendBuffer,usLen);
	memcpy(&chrDataToSend[usLen],chrSendBuffer,usLen);

	GetCommState(hComDev[ulChannelNo] ,&dcb);
	dcb.fDtrControl = 0;//DTR = 1;发送
	SetCommState(hComDev[ulChannelNo] ,&dcb);

	if ( WriteFile(hComDev[ulChannelNo],chrSendBuffer,usLen,&iR,&(stcWriteStatus[ulChannelNo])) || GetLastError() != ERROR_IO_PENDING  ) 
		return -1;
	dwRes = WaitForSingleObject(stcWriteStatus[ulChannelNo].hEvent,1000);
	Sleep(10);
	dcb.fDtrControl = 1;//DTR = 0;接收
	SetCommState(hComDev[ulChannelNo] ,&dcb);
	Sleep(10);

	if(dwRes != WAIT_OBJECT_0 || ! GetOverlappedResult(hComDev[ulChannelNo], &stcWriteStatus[ulChannelNo], &iR, FALSE))
		return 0;
	return 0;
}

DWORD WINAPI ReceiveCOMData(PVOID pParam)
{
	unsigned long uLen;
	unsigned long ulLen1;
	unsigned long ulLen2;
	DWORD	dwRes;
	COMSTAT Comstat;
	DWORD dwErrorFlags;
	char chrBuffer[iBufferSize]={0};
	unsigned long ulUARTBufferEndTemp=ulUARTBufferEnd[0];

	unsigned long ulComNumber = 0;
	memcpy(&ulComNumber,pParam,4);


	while (1)
	{
		if ( ! ReadFile(hComDev[ulComNumber],chrBuffer,iBufferSize-1,&uLen,&(stcReadStatus[ulComNumber])) )
		{
			dwRes = GetLastError() ;
			if ( dwRes != ERROR_IO_PENDING)
			{
				ClearCommError(hComDev[ulComNumber],&dwErrorFlags,&Comstat);
				continue;
			}

			WaitForSingleObject(stcReadStatus[ulComNumber].hEvent,INFINITE);
			if ( !GetOverlappedResult(hComDev[ulComNumber], &(stcReadStatus[ulComNumber]), &uLen, FALSE))
				continue;
			if(uLen <= 0)
				continue;
			if ( (ulUARTBufferEndTemp + uLen) > UARTBufferLength )
			{
				ulLen1 = UARTBufferLength - ulUARTBufferEndTemp;
				ulLen2 = uLen - ulLen1;
				if (ulLen1 > 0)
				{
					memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,ulLen1);
				}
				if (ulLen2 > 0)
				{
					memcpy((void *)&chrUARTBuffers[ulComNumber][0],(void *)(chrBuffer+ulLen1),ulLen2);
				}
				ulUARTBufferEndTemp = ulLen2;
			}
			else
			{
				memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,uLen);
				ulUARTBufferEndTemp+=uLen;	
			}

			if (  ulUARTBufferEndTemp == ulUARTBufferStart[ulComNumber])
			{
				printf("Error!");
			}
			else
			{
				ulUARTBufferEnd[ulComNumber] = ulUARTBufferEndTemp;
			}
			continue;
		}

		if(uLen <= 0)
			continue;
		if ( (ulUARTBufferEndTemp + uLen) > (UARTBufferLength) )
		{
			ulLen1 = UARTBufferLength - ulUARTBufferEndTemp;
			ulLen2 = uLen - ulLen1;
			if (ulLen1 > 0)
			{
				memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,ulLen1);
			}
			if (ulLen2 > 0)
			{
				memcpy((void *)&chrUARTBuffers[ulComNumber][0],(void *)(chrBuffer+ulLen1),ulLen2);
			}
			ulUARTBufferEndTemp = ulLen2;
		}
		else
		{
			memcpy((void *)&chrUARTBuffers[ulComNumber][ulUARTBufferEnd[ulComNumber]],(void *)chrBuffer,uLen);
			ulUARTBufferEndTemp+=uLen;	
		}

		if (  ulUARTBufferEndTemp== ulUARTBufferStart[ulComNumber])
		{
			printf("Error!");
		}
		else
		{
			ulUARTBufferEnd[ulComNumber] = ulUARTBufferEndTemp;
		}	

	}
	return 0;
}

signed char OpenCOMDevice(const unsigned long ulPortNo,const unsigned long ulBaundrate)
{
	DWORD dwThreadID,dwThreadParam;
	COMSTAT Comstat;
	DWORD dwErrorFlags;
	DWORD dwRes;
	DCB dcb;
	COMMTIMEOUTS comTimeOut;
	char PortName[10] = {'\\','\\','.','\\','C','O','M',0,0,0};//"\\\\.\\COM";
	char chrTemple[5]={0};

	if(ulPortNo >= TOTAL_PORT_NUM)
	{
		printf("\nerror: exceed the max com port num\n");
		return -1;
	}

	itoa(ulPortNo + START_PORT_NUM, chrTemple, 10);
	strcat_s(PortName, chrTemple);

	if((hComDev[ulPortNo] = CreateFile(PortName,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED ,NULL))==INVALID_HANDLE_VALUE)
	{
		dwRes=GetLastError();
		return -1;
	}
	ulComMask |= 1<<ulPortNo;

	SetupComm(hComDev[ulPortNo] ,iBufferSize,iBufferSize);
	GetCommState(hComDev[ulPortNo] ,&dcb);
		dcb.BaudRate = ulBaundrate;
	dcb.fParity = NOPARITY;
	dcb.ByteSize=8;
	dcb.fDtrControl = 1;//DTR = 0;接收
	dcb.fRtsControl = 0;//RTS = 0;接收
	dcb.StopBits=ONESTOPBIT;

	SetCommState(hComDev[ulPortNo] ,&dcb);
	ClearCommError(hComDev[ulPortNo] ,&dwErrorFlags,&Comstat);
	dwRes = GetLastError();

	comTimeOut.ReadIntervalTimeout = 5;				
	comTimeOut.ReadTotalTimeoutMultiplier = 10;		
	comTimeOut.ReadTotalTimeoutConstant = 100;		
	comTimeOut.WriteTotalTimeoutMultiplier = 5;		
	comTimeOut.WriteTotalTimeoutConstant = 5;		
	SetCommTimeouts(hComDev[ulPortNo] ,&comTimeOut);	

	stcWriteStatus[ulPortNo] .hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	stcReadStatus[ulPortNo] .hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	stcReadStatus[ulPortNo].Internal = 0;
	stcReadStatus[ulPortNo].InternalHigh = 0;
	stcReadStatus[ulPortNo].Offset = 0;
	stcReadStatus[ulPortNo].OffsetHigh = 0;
	dwThreadParam = ulPortNo;
	hCOMThread[dwThreadParam] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ReceiveCOMData,&dwThreadParam,0,&dwThreadID);
	SetThreadPriority(hCOMThread[ulPortNo],THREAD_PRIORITY_NORMAL);
	Sleep(200);

	return 0;

} 

signed char SetBaundrate(const unsigned long ulPortNo,const unsigned long ulBaundrate)
{

	DCB dcb;	
	GetCommState(hComDev[ulPortNo] ,&dcb);
	dcb.BaudRate = ulBaundrate;
	SetCommState(hComDev[ulPortNo] ,&dcb);
	return 0;

} 
void CloseCOMDevice()
{
	unsigned char i;
	for(i=0 ; i<sizeof(ulComMask)*8 ; i++)
	{
		if((ulComMask & (1<<i))==0)
			continue;
		ulUARTBufferEnd[i] = 0;ulUARTBufferStart[i]=0;
		TerminateThread(hCOMThread[i],0);
		WaitForSingleObject(hCOMThread[i],10000);
		PurgeComm(hComDev[i],PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		CloseHandle(stcReadStatus[i].hEvent);
		CloseHandle(stcWriteStatus[i].hEvent);
		CloseHandle(hComDev[i]);
	}
	ulComMask = 0;
}
