//--------------------------------------------------------------------------------------------------
#include "global.h"
#if(MFRC522 == true)
#include "mfrc522.h"
#endif
#if(MFRC630 == true)
#include "mfrc630.h"
#endif
//--------------------------------------------------------------------------------------------------
#define TEMPO_RESET_TRANSPONDER_MS  25000
//--------------------------------------------------------------------------------------------------
static struct timespec TimerLetturaTransponderMs;
//-------------------------------------------------------------------------------------------------
// MIFARE 1K - 1024 byte in 16 settori di 4 blocchi di 16 byte (blocco 4 di ogni settore contiene key A, key B e flag di accesso)
//-------------------------------------------------------------------------------------------------
const byte KEY_A[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };	// key_a default
const byte KEY_B[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };	// key_b default
//-------------------------------------------------------------------------------------------------
#define BLOCK_DATI              4
#define BLOCK_SEC_DATI          7
//-------------------------------------------------------------------------------------------------
typedef union
{
	struct
	{
		// invertite all'interno del bytre
		byte C1_b2_negato : 1;	// 8
		byte C1_b1_negato : 1;	// 7
		byte C1_b0_negato : 1;	// 6
		byte C1_b3_negato : 1;	// 5
		byte C2_b0_negato : 1;	// 4
		byte C2_b1_negato : 1;	// 3
		byte C2_b2_negato : 1;	// 2
		byte C2_b3_negato : 1;	// 1

		byte C3_b0_negato : 1;	// 8
		byte C3_b1_negato : 1;	// 7
		byte C3_b2_negato : 1;	// 6
		byte C3_b3_negato : 1;	// 5
		byte C1_b0 : 1;		// 4
		byte C1_b1 : 1;		// 3
		byte C1_b2 : 1;		// 2
		byte C1_b3 : 1;		// 1

		byte C2_b0 : 1;		// 8
		byte C2_b1 : 1;		// 7
		byte C2_b2 : 1;		// 6
		byte C2_b3 : 1;		// 5
		byte C3_b0 : 1;		// 4
		byte C3_b1 : 1;		// 3
		byte C3_b2 : 1;		// 2
		byte C3_b3 : 1;		// 1

		byte ByteUser;
	}Struttura;
	byte Bytes[4];
}TAccessBits;
//-------------------------------------------------------------------------------------------------
typedef struct
{
	unsigned int Accessbits_DataBlock0 : 3;
	unsigned int Accessbits_DataBlock1 : 3;
	unsigned int Accessbits_DataBlock2 : 3;
	unsigned int Accessbits_SectorTrailer : 3;
	unsigned int NotUsed : 4;
	byte ByteUser;
}TAccessBitsValue;
//-------------------------------------------------------------------------------------------------
/*static byte BitsToByte(byte C1, byte C2, byte C3)
{
	return (((C1 & 0x01) << 2) | ((C2 & 0x01) << 1) | ((C3 & 0x01)));
}*/
//-------------------------------------------------------------------------------------------------
static void ByteToBits(byte Valore, byte* C1, byte* C2, byte* C3)
{
	if (Valore & 0x04)
		*C1 = 1;
	else	*C1 = 0;
	if (Valore & 0x02)
		*C2 = 1;
	else	*C2 = 0;
	if (Valore & 0x01)
		*C3 = 1;
	else	*C3 = 0;
}
//-------------------------------------------------------------------------------------------------
/*static void ConvertAccessBitsToByte(TAccessBits AccessBits, TAccessBitsValue* AccessBitsValue)
{
	AccessBitsValue->Accessbits_DataBlock0 = BitsToByte(AccessBits.Struttura.C1_b0, AccessBits.Struttura.C2_b0, AccessBits.Struttura.C3_b0);
	AccessBitsValue->Accessbits_DataBlock1 = BitsToByte(AccessBits.Struttura.C1_b1, AccessBits.Struttura.C2_b1, AccessBits.Struttura.C3_b1);
	AccessBitsValue->Accessbits_DataBlock2 = BitsToByte(AccessBits.Struttura.C1_b2, AccessBits.Struttura.C2_b2, AccessBits.Struttura.C3_b2);
	AccessBitsValue->Accessbits_SectorTrailer = BitsToByte(AccessBits.Struttura.C1_b3, AccessBits.Struttura.C2_b3, AccessBits.Struttura.C3_b3);
	AccessBitsValue->ByteUser = AccessBits.Struttura.ByteUser;
}*/
//-------------------------------------------------------------------------------------------------
static void ConvertByteToAccessBits(TAccessBitsValue AccessBitsValue, TAccessBits* AccessBits)
{
	bool C1, C2, C3;
	ByteToBits(AccessBitsValue.Accessbits_DataBlock0, (byte*)&C1, (byte*)&C2, (byte*)&C3);
	AccessBits->Struttura.C1_b0 = C1;
	AccessBits->Struttura.C1_b0_negato = !C1;
	AccessBits->Struttura.C2_b0 = C2;
	AccessBits->Struttura.C2_b0_negato = !C2;
	AccessBits->Struttura.C3_b0 = C3;
	AccessBits->Struttura.C3_b0_negato = !C3;
	ByteToBits(AccessBitsValue.Accessbits_DataBlock1, (byte*)&C1, (byte*)&C2, (byte*)&C3);
	AccessBits->Struttura.C1_b1 = C1;
	AccessBits->Struttura.C1_b1_negato = !C1;
	AccessBits->Struttura.C2_b1 = C2;
	AccessBits->Struttura.C2_b1_negato = !C2;
	AccessBits->Struttura.C3_b1 = C3;
	AccessBits->Struttura.C3_b1_negato = !C3;
	ByteToBits(AccessBitsValue.Accessbits_DataBlock2, (byte*)&C1, (byte*)&C2, (byte*)&C3);
	AccessBits->Struttura.C1_b2 = C1;
	AccessBits->Struttura.C1_b2_negato = !C1;
	AccessBits->Struttura.C2_b2 = C2;
	AccessBits->Struttura.C2_b2_negato = !C2;
	AccessBits->Struttura.C3_b2 = C3;
	AccessBits->Struttura.C3_b2_negato = !C3;
	ByteToBits(AccessBitsValue.Accessbits_SectorTrailer, (byte*)&C1, (byte*)&C2, (byte*)&C3);
	AccessBits->Struttura.C1_b3 = C1;
	AccessBits->Struttura.C1_b3_negato = !C1;
	AccessBits->Struttura.C2_b3 = C2;
	AccessBits->Struttura.C2_b3_negato = !C2;
	AccessBits->Struttura.C3_b3 = C3;
	AccessBits->Struttura.C3_b3_negato = !C3;
	AccessBits->Struttura.ByteUser = AccessBitsValue.ByteUser;
}
//-------------------------------------------------------------------------------------------------
#define	BLOCK_RAB_WAB_IAB_DAB		0	// READ Key A/B		WRITE Key A/B	Increment Key A/B 	Decrement Key A/B
#define	BLOCK_RAB_W___I___D__		2	// READ Key A/B 	WRITE never		Increment Never 	Decrement Never
#define	BLOCK_RAB_W_B_I___D__		4	// READ Key A/B		WRITE Key B		Increment Never 	Decrement Never
#define	BLOCK_RAB_W_B_I_B_DAB		6	// READ Key A/B 	WRITE Key B		Increment Key B 	Decrement Key A/B
#define	BLOCK_RAB_W___I___DAB		1	// READ Key A/B 	WRITE Never		Increment Never 	Decrement Key A/B
#define	BLOCK_R_B_W_B_I___D__		3	// READ Key B 		WRITE Key B		Increment Never 	Decrement Never
#define	BLOCK_R_B_W___I___D__		5	// READ Key B 		WRITE Never		Increment Never 	Decrement Never
#define	BLOCK_R___W___I___D__		7	// READ Key Never 	WRITE Never		Increment Never 	Decrement Never
//-------------------------------------------------------------------------------------------------
#define ACCESS_BITS_000			0	// KEY A (Read Never, Write Key A)	Access bits (Read Key A, Write Never)	Key B (Read Key A, Write Key A)
#define ACCESS_BITS_010			2	// KEY A (Read Never, Write Never)	Access bits (Read Key A, Write Never)	Key B (Read Key A, Write Never)
#define ACCESS_BITS_100			4	// KEY A (Read Never, Write Key B)	Access bits (Read Key A/B, Write Never)	Key B (Read Never, Write Key B)
#define ACCESS_BITS_110			6	// KEY A (Read Never, Write Never)	Access bits (Read Key A/B, Write Never)	Key B (Read Never, Write Never)
#define ACCESS_BITS_001			1	// KEY A (Read Never, Write Key A)	Access bits (Read Key A, Write Key A)	Key B (Read Key A, Write Key A)
#define ACCESS_BITS_011			3	// KEY A (Read Never, Write Key B)	Access bits (Read Key A/B, Write Key B)	Key B (Read Never, Write Key B)
#define ACCESS_BITS_101			5	// KEY A (Read Never, Write Never)	Access bits (Read Key A/B, Write Key B)	Key B (Read Never, Write Never)
#define ACCESS_BITS_111			7	// KEY A (Read Never, Write Never)	Access bits (Read Key A/B, Write Never)	Key B (Read Never, Write Never)
//-------------------------------------------------------------------------------------------------
static void CalcolaKeyA(MIFARE_Key* key)
{
	key->keyByte[0] = KEY_A[0];
	key->keyByte[1] = KEY_A[1];
	key->keyByte[2] = KEY_A[2];
	key->keyByte[3] = KEY_A[3];
	key->keyByte[4] = KEY_A[4];
	key->keyByte[5] = KEY_A[5];
}
//-------------------------------------------------------------------------------------------------
static void CalcolaKeyB(MIFARE_Key* key)
{
	key->keyByte[0] = KEY_B[0];
	key->keyByte[1] = KEY_B[1];
	key->keyByte[2] = KEY_B[2];
	key->keyByte[3] = KEY_B[3];
	key->keyByte[4] = KEY_B[4];
	key->keyByte[5] = KEY_B[5];
}
//-------------------------------------------------------------------------------------------------
byte IsTesseraPresente(void)
{
	static byte TesseraPresente = false, OldTesseraPresente = false;
	if (CheckTimeoutExpired(TimerLetturaTransponderMs) == 1)
	{
		pthread_mutex_lock(&mutex_transp);
#if(MFRC522 == true)
		// interrogo scheda ogni 250ms - circa 2 ms se no carta, circa 64 ms se carta presente
		PCD_AntennaOn();
		TesseraPresente = false;
		if (PICC_IsNewCardPresent() == true)
		{
			if (PICC_ReadCardSerial() == true)
			{
				TesseraPresente = true;
			}
		}
		PICC_ReadCardSerial();		// TRUCCO PER RESETTARE LETTURA NUOVA CARD
		PICC_HaltA();               // Halt PICC
		PCD_StopCrypto1();          // Stop encryption on PCD
#endif
#if(MFRC630 == true)
		// interrogo scheda ogni 250ms - circa 20ms
		Uid uid;
		TesseraPresente = false;
		if (mfrc630_MF_ready(&uid))
		{
			TesseraPresente = true;
		}
#endif
		pthread_mutex_unlock(&mutex_transp);
		OldTesseraPresente = TesseraPresente;
		SetTimeout(&TimerLetturaTransponderMs, 250);
	}
	return OldTesseraPresente;
}
//-------------------------------------------------------------------------------------------------
byte IsLeggiDatiTransponder(void)
{
	pthread_mutex_lock(&mutex_transp);
#if(MFRC522 == true)
	MIFARE_Key key;
	byte size, i, blockaddr, BufferAreaDati[18];
	byte TesseraPresente = false;
	if (PICC_IsNewCardPresent() == true)
	{
		if (PICC_ReadCardSerial() == true)
		{
			TesseraPresente = true;
			for (i = 0; i < MAX_DIM_UID; i++)
				BloccoDatiIn.Struttura.Uid[i] = 0;
			for (i = 0; i < mfrc522.uid.size; i++)
				BloccoDatiIn.Struttura.Uid[i] = mfrc522.uid.uidByte[i];
			for (i = 0; i < DIM_CODICE_TESSERA; i++)
				BloccoDatiIn.Struttura.CodiceTessera[i] = 0;
			BloccoDatiIn.Struttura.DimUidTessera = mfrc522.uid.size;
			if (mfrc522.uid.size == DIM_UID_CLASSIC)
			{
				// se è un mifare classic prova a leggere il codice tessera

				CalcolaKeyA(&key);
				// lettura area dati
				if (PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, BLOCK_SEC_DATI, &key, &(mfrc522.uid)) == STATUS_OK)
				{
					size = sizeof(BufferAreaDati);
					blockaddr = BLOCK_DATI;
					if (MIFARE_Read(blockaddr, BufferAreaDati, &size) == STATUS_OK)
					{
						if (VERIFICA_CARD == true) {
							if ((BufferAreaDati[0] == CHECK_CAR) &&
								(BufferAreaDati[1] == START_CAR) &&
								(BufferAreaDati[DIM_CODICE_TESSERA * 2] == STOP_CAR))
							{
								for (i = 0; i < DIM_CODICE_TESSERA; i++)
									BloccoDatiIn.Struttura.CodiceTessera[i] = NibbleToInt(BufferAreaDati[1 + i * 2], BufferAreaDati[2 + i * 2]);
							}
							else
							{
								for (i = 0; i < DIM_CODICE_TESSERA; i++)
									BloccoDatiIn.Struttura.CodiceTessera[i] = 0;
							}
						}
						else
						{
							for (i = 0; i < DIM_CODICE_TESSERA; i++)
								BloccoDatiIn.Struttura.CodiceTessera[i] = BufferAreaDati[1 + i];
						}
					}
				}
			}
		}
	}
	PICC_ReadCardSerial();		// TRUCCO PER RESETTARE LETTURA NUOVA CARD
	PICC_HaltA();               // Halt PICC
	PCD_StopCrypto1();          // Stop encryption on PCD
#endif
#if(MFRC630 == true)
	Uid uid;
	MIFARE_Key key;
	byte size, i, diffzero, BufferAreaDati[18];
	byte TesseraPresente = false, fine, numretray;
	if (mfrc630_MF_ready(&uid))
	{
		for (i = 0; i < MAX_DIM_UID; i++)
			BloccoDatiIn.Struttura.Uid[i] = 0;
		diffzero = false;
		for (i = 0; i < uid.size; i++)
		{
			BloccoDatiIn.Struttura.Uid[i] = uid.uidByte[i];
			if (uid.uidByte[i] != 0)
				diffzero = true;
		}
		for (i = 0; i < DIM_CODICE_TESSERA; i++)
			BloccoDatiIn.Struttura.CodiceTessera[i] = 0;
		if (diffzero == true)
		{
			BloccoDatiIn.Struttura.DimUidTessera = uid.size;
			if (ONLY_UID == true)
			{
				// se solo uid considero subito tessera presente e non leggo contenuto tesssera
				TesseraPresente = true;
			}
			else
			{
				// altrimenti considera valida solo se riesce a leggere contenuto valido
				if (uid.size == DIM_UID_CLASSIC)
				{
					// tenta lettura tessera per 3 volte
					numretray = 0;
					fine = false;
					while (fine == false)
					{
						// se è un mifare classic prova a leggere il codice tessera
						CalcolaKeyA(&key);
						// lettura area dati
						mfrc630_cmd_load_key(key.keyByte);  // load into the key buffer
						if (mfrc630_MF_auth(uid.uidByte, MFRC630_MF_AUTH_KEY_A, BLOCK_SEC_DATI))
						{
							size = mfrc630_MF_read_block(BLOCK_DATI, BufferAreaDati);
							if (size == 16)
							{
								if (VERIFICA_CARD == true) {
									if ((BufferAreaDati[0] == CHECK_CAR) &&
										(BufferAreaDati[1] == START_CAR) &&
										(BufferAreaDati[DIM_CODICE_TESSERA * 2] == STOP_CAR))
									{
										for (i = 0; i < DIM_CODICE_TESSERA; i++)
											BloccoDatiIn.Struttura.CodiceTessera[i] = NibbleToInt(BufferAreaDati[1 + i * 2], BufferAreaDati[2 + i * 2]);
									}
									else
									{
										for (i = 0; i < DIM_CODICE_TESSERA; i++)
											BloccoDatiIn.Struttura.CodiceTessera[i] = 0;
									}
								}
								else
								{
									for (i = 0; i < DIM_CODICE_TESSERA; i++)
										BloccoDatiIn.Struttura.CodiceTessera[i] = BufferAreaDati[1 + i];
								}
							}
						}
						diffzero = false;
						for (i = 0; i < DIM_CODICE_TESSERA; i++)
						{
							if (BloccoDatiIn.Struttura.CodiceTessera[i] != 0)
								diffzero = true;
						}
						if (diffzero == true)
						{
							TesseraPresente = true;
							fine = true;
						}
						else
						{
							numretray++;
							if (numretray > 3)
								fine = true;
						}
						mfrc630_MF_deauth();  // be sure to call this after an authentication!
					}
				}
			}
		}
		mfrc630_MF_deauth();  // be sure to call this after an authentication!
	}
#endif
	pthread_mutex_unlock(&mutex_transp);
	return TesseraPresente;
}
//-------------------------------------------------------------------------------------------------
bool IsWriteDatiTransponder(uint8_t BufferDati[])
{
	pthread_mutex_lock(&mutex_transp);
	MIFARE_Key key;
	TAccessBits AccessBits;
	TAccessBitsValue AccessBitsValues;
	byte i, blockaddr, BufferAreaDati[18];
	byte OkAreaDati = false;
#if(MFRC522 == true)
	if (PICC_IsNewCardPresent() == true)
	{
		if (PICC_ReadCardSerial() == true)
		{
			CalcolaKeyA(&key);
			// lettura area dati
			if (PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, BLOCK_SEC_DATI, &key, &(mfrc522.uid)) == STATUS_OK)
			{
				// scrive sector trailer
				for (i = 0; i < 6; i++)
					BufferAreaDati[i] = KEY_A[i];
				AccessBitsValues.Accessbits_DataBlock0 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
				AccessBitsValues.Accessbits_DataBlock1 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
				AccessBitsValues.Accessbits_DataBlock2 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
				AccessBitsValues.Accessbits_SectorTrailer = ACCESS_BITS_011;             // KEY A (Read Never, Write Key B), Access bits (Read Key A/B, Write Key B), Key B (Read Never, Write Key B)
				AccessBitsValues.ByteUser = 0x69;
				ConvertByteToAccessBits(AccessBitsValues, &AccessBits);
				BufferAreaDati[6] = AccessBits.Bytes[0];
				BufferAreaDati[7] = AccessBits.Bytes[1];
				BufferAreaDati[8] = AccessBits.Bytes[2];
				BufferAreaDati[9] = AccessBits.Bytes[3];
				for (i = 0; i < 6; i++)
					BufferAreaDati[10 + i] = KEY_B[i];
				// anche se non va a buon fine va avanti comunque
				MIFARE_Write(BLOCK_SEC_DATI, BufferAreaDati, 16);

				//size = sizeof(BufferAreaDati);
				blockaddr = BLOCK_DATI;
				BufferAreaDati[0] = CHECK_CAR;
				for (i = 0; i < NUM_BYTE_TESSERA; i++)
					BufferAreaDati[i + 1] = BufferDati[i];
				if (MIFARE_Write(blockaddr, BufferAreaDati, 16) == STATUS_OK)
				{
					OkAreaDati = true;
				}
			}
		}
	}
	PICC_ReadCardSerial();		// TRUCCO PER RESETTARE LETTURA NUOVA CARD
	PICC_HaltA();			// Halt PICC
	PCD_StopCrypto1();		// Stop encryption on PCD
	if (OkAreaDati == false)
	{
		// se errore provo a scrivere con key b perchè alcune tessere sono inizializzate con la scrittura solo su key b
		if (PICC_IsNewCardPresent() == true)
		{
			if (PICC_ReadCardSerial() == true)
			{
				CalcolaKeyB(&key);
				// lettura area dati
				if (PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_B, BLOCK_SEC_DATI, &key, &(mfrc522.uid)) == STATUS_OK)
				{
					// scrive sector trailer
					for (i = 0; i < 6; i++)
						BufferAreaDati[i] = KEY_A[i];
					AccessBitsValues.Accessbits_DataBlock0 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
					AccessBitsValues.Accessbits_DataBlock1 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
					AccessBitsValues.Accessbits_DataBlock2 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
					AccessBitsValues.Accessbits_SectorTrailer = ACCESS_BITS_011;             // KEY A (Read Never, Write Key B), Access bits (Read Key A/B, Write Key B), Key B (Read Never, Write Key B)
					AccessBitsValues.ByteUser = 0x69;
					ConvertByteToAccessBits(AccessBitsValues, &AccessBits);
					BufferAreaDati[6] = AccessBits.Bytes[0];
					BufferAreaDati[7] = AccessBits.Bytes[1];
					BufferAreaDati[8] = AccessBits.Bytes[2];
					BufferAreaDati[9] = AccessBits.Bytes[3];
					for (i = 0; i < 6; i++)
						BufferAreaDati[10 + i] = KEY_B[i];
					// anche se non va a buon fine va avanti comunque
					MIFARE_Write(BLOCK_SEC_DATI, BufferAreaDati, 16);

					// scrive area dati
					//size = sizeof(BufferAreaDati);
					blockaddr = BLOCK_DATI;
					BufferAreaDati[0] = CHECK_CAR;
					for (i = 0; i < NUM_BYTE_TESSERA; i++)
						BufferAreaDati[i + 1] = BufferDati[i];
					if (MIFARE_Write(blockaddr, BufferAreaDati, 16) == STATUS_OK)
					{
						OkAreaDati = true;
					}
				}
			}
		}
		PICC_ReadCardSerial();		// TRUCCO PER RESETTARE LETTURA NUOVA CARD
		PICC_HaltA();			// Halt PICC
		PCD_StopCrypto1();		// Stop encryption on PCD
	}
#endif
#if(MFRC630 == true)
	Uid uid;
	if (mfrc630_MF_ready(&uid))
	{
		CalcolaKeyA(&key);
		// lettura area dati
		mfrc630_cmd_load_key(key.keyByte);  // load into the key buffer
		if (mfrc630_MF_auth(uid.uidByte, MFRC630_MF_AUTH_KEY_A, BLOCK_SEC_DATI))
		{
			// scrive sector trailer
			for (i = 0; i < 6; i++)
				BufferAreaDati[i] = KEY_A[i];
			AccessBitsValues.Accessbits_DataBlock0 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
			AccessBitsValues.Accessbits_DataBlock1 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
			AccessBitsValues.Accessbits_DataBlock2 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
			AccessBitsValues.Accessbits_SectorTrailer = ACCESS_BITS_011;             // KEY A (Read Never, Write Key B), Access bits (Read Key A/B, Write Key B), Key B (Read Never, Write Key B)
			AccessBitsValues.ByteUser = 0x69;
			ConvertByteToAccessBits(AccessBitsValues, &AccessBits);
			BufferAreaDati[6] = AccessBits.Bytes[0];
			BufferAreaDati[7] = AccessBits.Bytes[1];
			BufferAreaDati[8] = AccessBits.Bytes[2];
			BufferAreaDati[9] = AccessBits.Bytes[3];
			for (i = 0; i < 6; i++)
				BufferAreaDati[10 + i] = KEY_B[i];
			// anche se non va a buon fine va avanti comunque                                        
			mfrc630_MF_write_block(BLOCK_SEC_DATI, BufferAreaDati);

			blockaddr = BLOCK_DATI;
			BufferAreaDati[0] = CHECK_CAR;
			for (i = 0; i < NUM_BYTE_TESSERA; i++)
				BufferAreaDati[i + 1] = BufferDati[i];
			if (mfrc630_MF_write_block(blockaddr, BufferAreaDati) == 16)
			{
				OkAreaDati = true;
			}
		}
		mfrc630_MF_deauth();  // be sure to call this after an authentication!                   		
	}
	if (OkAreaDati == false)
	{
		// se errore provo a scrivere con key b perchè alcune tessere sono inizializzate con la scrittura solo su key b
		if (mfrc630_MF_ready(&uid))
		{
			CalcolaKeyB(&key);
			// lettura area dati
			mfrc630_cmd_load_key(key.keyByte);  // load into the key buffer
			if (mfrc630_MF_auth(uid.uidByte, MFRC630_MF_AUTH_KEY_B, BLOCK_SEC_DATI))
			{
				// scrive sector trailer
				for (i = 0; i < 6; i++)
					BufferAreaDati[i] = KEY_A[i];
				AccessBitsValues.Accessbits_DataBlock0 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
				AccessBitsValues.Accessbits_DataBlock1 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
				AccessBitsValues.Accessbits_DataBlock2 = BLOCK_RAB_W_B_I_B_DAB;        // READ Key A/B, WRITE Key B, Increment Key B, Decrement Key A/B
				AccessBitsValues.Accessbits_SectorTrailer = ACCESS_BITS_011;             // KEY A (Read Never, Write Key B), Access bits (Read Key A/B, Write Key B), Key B (Read Never, Write Key B)
				AccessBitsValues.ByteUser = 0x69;
				ConvertByteToAccessBits(AccessBitsValues, &AccessBits);
				BufferAreaDati[6] = AccessBits.Bytes[0];
				BufferAreaDati[7] = AccessBits.Bytes[1];
				BufferAreaDati[8] = AccessBits.Bytes[2];
				BufferAreaDati[9] = AccessBits.Bytes[3];
				for (i = 0; i < 6; i++)
					BufferAreaDati[10 + i] = KEY_B[i];
				// anche se non va a buon fine va avanti comunque       
				mfrc630_MF_write_block(BLOCK_SEC_DATI, BufferAreaDati);

				// scrive area dati
				blockaddr = BLOCK_DATI;
				BufferAreaDati[0] = CHECK_CAR;
				for (i = 0; i < NUM_BYTE_TESSERA; i++)
					BufferAreaDati[i + 1] = BufferDati[i];
				if (mfrc630_MF_write_block(blockaddr, BufferAreaDati) == 16)
				{
					OkAreaDati = true;
				}
			}
			mfrc630_MF_deauth();  // be sure to call this after an authentication!                   		
		}
	}
#endif
	pthread_mutex_unlock(&mutex_transp);
	return OkAreaDati;
}
//-------------------------------------------------------------------------------------------------
void* GestTransponder(void* args) {
	struct timespec TimerResetTransponderMs;
	uint8_t i = 0, OldTesseraPresente = false;
	char tempstr[10];
	char log_str[256];
	static uint8_t TesseraOk = 0;

	delay(1);
	LOG_I((char*)"Inizializza Transponder");
	SetTimeout(&TimerLetturaTransponderMs, 1);
	pthread_mutex_lock(&mutex_transp);
#if(MFRC522 == true)
	PCD_Init(); 		// Init MFRC522 card
#endif
#if(MFRC630 == true)
	uint8_t init_ok = false;
	while (init_ok == false)
	{
		if (mfrc630_iso_14443A_init() == true)
			init_ok = true;
		else
		{
			LOG_E((char*)"Transponder NON Ok!!!");
			delay(3000);
		}
	}
#endif
	pthread_mutex_unlock(&mutex_transp);
	LOG_I((char*)"Transponder Ok\r\n");

	/*
		char str[128], tempstr[10];
		while(true)
		{
			if(PICC_IsNewCardPresent() == true)	// card presente
			{
				strcpy(str, "1");
				if(PICC_ReadCardSerial() == true)
				{
					strcat(str," ");
					for(i=0;i<mfrc522.uid.size;i++)
					{
					sprintf(tempstr, "%02x", mfrc522.uid.uidByte[i]);
					strcat(str, tempstr);;
					}
				}
				PICC_ReadCardSerial();
			}
			else	strcat(str, "0");
			strcat(str, "Ant:");
			sprintf(tempstr, "%02x", PCD_GetAntennaGain());
			strcat(str, tempstr);
			LOG_I(str);
			PICC_HaltA();		// Halt PICC
			PCD_StopCrypto1();	// Stop encryption on PCD
			sleep(1);
		}*/
	OldTesseraPresente = !IsTesseraPresente();
	SetTimeout(&TimerResetTransponderMs, TEMPO_RESET_TRANSPONDER_MS);
	while (true)
	{
		delay(25);
		if (IsTesseraPresente() != OldTesseraPresente)
		{
			TesseraOk = false;
			OldTesseraPresente = IsTesseraPresente();
			SetTimeout(&TimerResetTransponderMs, TEMPO_RESET_TRANSPONDER_MS);

			if (OldTesseraPresente == true)
			{
				//prova 10 volte lettura tessera
				for (i = 0; i < 10; i++)
				{
					if (IsTesseraPresente() == true)
					{
						if (IsLeggiDatiTransponder() == true)
						{
							SetTimeout(&TimerResetTransponderMs, TEMPO_RESET_TRANSPONDER_MS);
							TesseraOk = true;
							BloccoDatiIn.Struttura.FlagTesseraPresente = true;
							strcpy(log_str, "Uid: ");
							strcpy(DataToSend, "Rfid ");
							for (i = 0; i < BloccoDatiIn.Struttura.DimUidTessera; i++)
							{
								sprintf(tempstr, "%02X", BloccoDatiIn.Struttura.Uid[i]);
								strcat(DataToSend, tempstr);
								strcat(log_str, tempstr);
							}
							strcat(log_str, "   Codice: ");
							strcat(DataToSend, " ");
							for (i = 0; i < DIM_CODICE_TESSERA; i++)
							{
								sprintf(tempstr, "%02X", BloccoDatiIn.Struttura.CodiceTessera[i]);
								strcat(DataToSend, tempstr);
								strcat(log_str, tempstr);
							}
							LOG_I(log_str);
							ComponiPost(DataToSend);
							FlagNewTessera = true;
							BeepTessera();
						}
						i = 10;
					}
					else
					{
						delay(25);
					}
				}
			}
			if (TesseraOk == true)
				BloccoDatiIn.Struttura.FlagTesseraPresente = true;
			else BloccoDatiIn.Struttura.FlagTesseraPresente = false;
			if (TesseraOk == false)
			{
				// se tessera non presente invia tutti 0
				for (i = 0; i < MAX_DIM_UID; i++)
					BloccoDatiIn.Struttura.Uid[i] = 0x00;
				for (i = 0; i < DIM_CODICE_TESSERA; i++)
					BloccoDatiIn.Struttura.CodiceTessera[i] = 0x00;
				OldTesseraPresente = false; // azzera old tessera in modo da ritentare lettura
			}
		}
		if (TesseraOk == false)
		{
			// ogni 25 secondi reset lettore transponder se tessera non presente
			if (CheckTimeoutExpired(TimerResetTransponderMs) == 1)
			{
				SetTimeout(&TimerResetTransponderMs, TEMPO_RESET_TRANSPONDER_MS);
				pthread_mutex_lock(&mutex_transp);
				PCD_ReInit();           // 160 ms
				pthread_mutex_unlock(&mutex_transp);
			}
		}
		else
		{
			if (FlagNewWrite == true)
			{
				FlagNewWrite = false;
				strcpy(log_str, "Write: ");
				for (i = 0; i < NUM_BYTE_TESSERA; i++)
				{
					char tempstr[10];
					sprintf(tempstr, "%02x", BufferWrite[i]);
					strcpy(log_str, tempstr);
				}
				LOG_I(log_str);
				if (IsWriteDatiTransponder(BufferWrite) == true)
				{
					LOG_I((char*)"Write Ok!");
				}
				else
				{
					LOG_W((char*)"Write NON OK!!!");
				}
				OldTesseraPresente = FALSE;     // dopo scrittura ritenta lettura
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------