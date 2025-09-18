#include "global.h"
//--------------------------------------------------------------------------------------------------
pthread_mutex_t mutex_transp;
TBloccoDatiInput BloccoDatiIn;
bool FlagNewTessera, FlagNewWrite;
uint8_t BufferWrite[NUM_BYTE_TESSERA];
char DataToSend[250];
//--------------------------------------------------------------------------------------------------
void SetTimeout(struct timespec* timeout, int32_t timeout_ms)
{
	int32_t val_nsec;
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	val_nsec = (now.tv_nsec + ((timeout_ms % 1000) * 1000000));
	if (val_nsec >= 1000000000)
	{
		timeout->tv_nsec = (val_nsec % 1000000000);
		timeout->tv_sec = (now.tv_sec + 1 + (timeout_ms / 1000));
	}
	else
	{
		timeout->tv_nsec = val_nsec;
		timeout->tv_sec = now.tv_sec + (timeout_ms / 1000);
	}
}
//--------------------------------------------------------------------------------------------------
int8_t CheckTimeoutExpired(struct timespec timeout)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	if (now.tv_sec > timeout.tv_sec)
	{
		return 1;
	}
	else
	{
		if (now.tv_sec == timeout.tv_sec)
		{
			if (now.tv_nsec >= timeout.tv_nsec)
			{
				return 1;
			}
		}
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------
void ResetDatiInput(void)
{
	uint8_t i;
	for (i = 0; i < DIM_BLOCCO_DATI_IN; i++)
	{
		BloccoDatiIn.Buffer[i] = 0;
	}
}
//--------------------------------------------------------------------------------------------------
uint8_t NibbleToInt(uint8_t nh, uint8_t nl)
{
	uint8_t valh, vall;
	if ((nh >= '0') && (nh <= '9'))
	{
		valh = uint8_t(nh - '0');
	}
	else
	{
		if ((nh >= 'A') && (nh <= 'F'))
		{
			valh = uint8_t(nh - 'A' + 10);
		}
		else if ((nh >= 'a') && (nh <= 'f'))
		{
			valh = uint8_t(nh - 'a' + 10);
		}
		else valh = 0;
	}
	if ((nl >= '0') && (nl <= '9'))
	{
		vall = uint8_t(nl - '0');
	}
	else
	{
		if ((nl >= 'A') && (nl <= 'F'))
		{
			vall = uint8_t(nl - 'A' + 10);
		}
		else if ((nl >= 'a') && (nl <= 'f'))
		{
			vall = uint8_t(nl - 'a' + 10);
		}
		else vall = 0;
	}
	return uint8_t((valh << 4) + vall);
}
//-------------------------------------------------------------------------------------------------
void BeepTessera(void)
{
	digitalWrite(BUZZER, LOW);
	delay(250);
	digitalWrite(BUZZER, HIGH);
}
//--------------------------------------------------------------------------------------------------
