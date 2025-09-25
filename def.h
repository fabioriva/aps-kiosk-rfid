//------------------------------------------------------------------------------
// tipo di transponder (uno SOLO a true)
#define MFRC522				false
#define MFRC630				true
//------------------------------------------------------------------------------
#define ONLY_UID			true	// =true legge solo uid, =false contenuto tessera
#define VERIFICA_CARD		false	// se ONLY_UID=false: =true verifica che sia tessera formattata, =false restituisce sempre il contenuto
//------------------------------------------------------------------------------
#define LOG_I_ENABLED		true
#define LOG_W_ENABLED		true
#define LOG_E_ENABLED		true
//------------------------------------------------------------------------------
#define DELAY_NOP			25		// delay per nop (25 per RPI4, 8 per RPI0)
//------------------------------------------------------------------------------
#define MAX_DIM_UID			8
#define DIM_UID_CLASSIC     4       // uid mifare classic 1K, 4K
#define DIM_UID_DESFIRE     7		// dim. uid mifare desfire
//------------------------------------------------------------------------------
#define DIM_CODICE_TESSERA	4
#define NUM_BYTE_TESSERA	(DIM_CODICE_TESSERA*2)
#define CHECK_CAR           0xC9
#define START_CAR           'F'
#define STOP_CAR            'E'
//------------------------------------------------------------------------------
/*// hw tea
// utilizzare "gpio readall" per vedere stato e numeri io(BCM=GPIO, physical=pin)
#define SPI_SCK				0       // GPIO17=0     pin 11
#define SPI_MISO			2       // GPIO27=2     pin 13
#define SPI_MOSI			3       // GPIO22=3     pin 15
#define SPI_CS				4       // GPIO23=4     pin 16
#define CHIP_POWER			26      // GPIO12=26    pin 32
#define LED					22      // GPIO6=22     pin 31
#define BUZZER				25      // GPIO26=25    pin 37
*/
#define SPI_SCK				14      // GPIO11=14     pin 23
#define SPI_MISO			13      // GPIO09=13     pin 21
#define SPI_MOSI			12      // GPIO10=12     pin 19
#define SPI_CS				10      // GPIO08=10     pin 24
#define CHIP_POWER			6       // GPIO25=6      pin 22
#define BUZZER				8       // GPIO02=8      pin 3
//------------------------------------------------------------------------------
#define API_URL "http://localhost/api/kiosk/tag"
#define API_PORT 9999
//------------------------------------------------------------------------------
