// UARTTest.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//

#include "Com.h"
#include "windows.h"
#include "time.h"
#include "stdio.h"
#include "JY901.h"
unsigned char ucComNo[2] ={0,0};

int main(int argc, char* argv[])
{
	char chrBuffer[2000];
	unsigned short usLength=0,usCnt=0;
	unsigned long ulBaund= 460800,ulComNo=3;
	signed char cResult= 1;	
	//printf("Input com port:\r\nCom = ");
	//scanf("%ld", &ulComNo);
	//printf("Input baud rate:(9600/14400/19200/38400/57600/115200/239400/460800/921600)\r\nBaud = "); //921600 is likely to be unstable
	//scanf("%ld", &ulBaund);
	//printf("waiting to open serial port %d...\r\n", ucComNo);
	while(cResult!=0)
	{
		cResult = OpenCOMDevice(ulComNo,ulBaund);
	}

	while(1)
	{

		usLength = CollectUARTData(ulComNo,chrBuffer);
		if (usLength>0)
		{
			JY901.CopeSerialData(chrBuffer,usLength);
		}
		//Sleep(100);
		
		if (usCnt++>=0)
		{
			usCnt=0;
			//printf("Time:20%d-%d-%d %d:%d:%.3f\r\n",(short)JY901.stcTime.ucYear,(short)JY901.stcTime.ucMonth,
			//		(short)JY901.stcTime.ucDay,(short)JY901.stcTime.ucHour,(short)JY901.stcTime.ucMinute,(float)JY901.stcTime.ucSecond+(float)JY901.stcTime.usMiliSecond/1000);

			printf("Acc:%.3f %.3f %.3f\r\n",(float)JY901.stcAcc.a[0]/32768*16,(float)JY901.stcAcc.a[1]/32768*16,(float)JY901.stcAcc.a[2]/32768*16);

			printf("Gyro:%.3f %.3f %.3f\r\n",(float)JY901.stcGyro.w[0]/32768*2000,(float)JY901.stcGyro.w[1]/32768*2000,(float)JY901.stcGyro.w[2]/32768*2000);

			printf("Angle:%.3f %.3f %.3f\r\n",(float)JY901.stcAngle.Angle[0]/32768*180,(float)JY901.stcAngle.Angle[1]/32768*180,(float)JY901.stcAngle.Angle[2]/32768*180);

			printf("Mag:%d %d %d\r\n",JY901.stcMag.h[0],JY901.stcMag.h[1],JY901.stcMag.h[2]);

			//printf("Pressure:%lx Height%.2f\r\n",JY901.stcPress.lPressure,(float)JY901.stcPress.lAltitude/100);

			//printf("DStatus:%d %d %d %d\r\n",JY901.stcDStatus.sDStatus[0],JY901.stcDStatus.sDStatus[1],JY901.stcDStatus.sDStatus[2],JY901.stcDStatus.sDStatus[3]);

			//printf("Longitude:%ldDeg%.5fm Lattitude:%ldDeg%.5fm\r\n",JY901.stcLonLat.lLon/10000000,(double)(JY901.stcLonLat.lLon % 10000000)/1e5,JY901.stcLonLat.lLat/10000000,(double)(JY901.stcLonLat.lLat % 10000000)/1e5);

			//printf("GPSHeight:%.1fm GPSYaw:%.1fDeg GPSV:%.3fkm/h\r\n\r\n",(float)JY901.stcGPSV.sGPSHeight/10,(float)JY901.stcGPSV.sGPSYaw/10,(float)JY901.stcGPSV.lGPSVelocity/1000);

			printf("\r\n");
		}	
		
	}
		return 0;
}

