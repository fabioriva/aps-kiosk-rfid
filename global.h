//#ifndef GLOBAL_H
//#define GLOBAL_H
//------------------------------------------------------------------------------
#include <wiringPi.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "gesttransponder.h"
//------------------------------------------------------------------------------
extern void SetTimeout(struct timespec* timeout, int32_t timeout_ms);
extern int8_t CheckTimeoutExpired(struct timespec timeout);
extern uint8_t NibbleToInt(uint8_t nh, uint8_t  nl);
extern void ResetDatiInput(void);
extern void BeepTessera(void);
extern void ComponiPost(char *content);
extern int http_post(const char* url, uint16_t portno, const char* data);
extern int http_get(const char* host, uint16_t port, const char* params);
//------------------------------------------------------------------------------
#define DIM_BLOCCO_DATI_IN      32
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint8_t Uid[MAX_DIM_UID];
		uint8_t CodiceTessera[DIM_CODICE_TESSERA];
		uint8_t FlagTesseraPresente : 1;
		uint8_t NotUsed : 7;
		uint8_t DimUidTessera;
	}Struttura;
	uint8_t Buffer[DIM_BLOCCO_DATI_IN];
}TBloccoDatiInput;
//------------------------------------------------------------------------------
extern pthread_mutex_t mutex_transp;
extern TBloccoDatiInput BloccoDatiIn;
extern bool FlagNewTessera, FlagNewWrite;
extern uint8_t BufferWrite[NUM_BYTE_TESSERA];
extern char DataToSend[250];
//------------------------------------------------------------------------------
//#endif /* GLOBAL_H */
