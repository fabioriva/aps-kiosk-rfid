//--------------------------------------------------------------------------------------------------
#include "global.h"
//--------------------------------------------------------------------------------------------------
int main(void)
{
	pthread_t id;
	int ret;

	wiringPiSetup();

	pinMode(BUZZER, OUTPUT);
	digitalWrite(BUZZER, HIGH); // buzzer attivo basso

	ResetDatiInput();

	ret = pthread_create(&id, NULL, &GestTransponder, NULL);
	if (ret != 0) {
		LOG_E((char*)"Thread Transponder Not created!");
	}

	FlagNewTessera = false;
	FlagNewWrite = false;
	BufferWrite[0] = START_CAR;
	BufferWrite[1] = '0';
	BufferWrite[2] = '5';
	BufferWrite[3] = '0';
	BufferWrite[4] = '0';
	BufferWrite[5] = '6';
	BufferWrite[6] = '6';
	BufferWrite[7] = STOP_CAR;

	while (true)
	{
		if (FlagNewTessera == true)
		{
			char log_str[128];
			strcpy(log_str, "Send:");
			strcat(log_str, DataToSend);
			LOG_I(log_str);
			//http_get(API_URL, API_PORT, DataToSend);
			http_post(API_URL, API_PORT, DataToSend);
			FlagNewTessera = false;
		}
		delay(500);					// ms
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------
