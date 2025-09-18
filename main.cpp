
// trovare corrispondenza pin
// provare scrittura

//--------------------------------------------------------------------------------------------------
#include "global.h"
//--------------------------------------------------------------------------------------------------
int main(void)
{
	pthread_t id;
	int ret;

	wiringPiSetup();

	pinMode(BUZZER, OUTPUT);
	digitalWrite(BUZZER, HIGH); // buzzer attivo alto

	ResetDatiInput();

	ret = pthread_create(&id, NULL, &GestTransponder, NULL);
	if (ret != 0) {
		printf("Thread Transponder Not created!\r\n");
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
			printf("Send: %s\n", DataToSend);
			//http_get("127.0.0.1", 8080, DataToSend);
			http_post(API_URL, API_PORT, DataToSend);
			FlagNewTessera = false;
		}
		delay(500);					// ms
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------
