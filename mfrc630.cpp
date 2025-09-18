
#include "global.h"
#if(MFRC630 == true)
#include "mfrc630.h"
//--------------------------------------------------------------------------------------------------
static void Nop(void)
{
	uint8_t i = 0;
	while (i < DELAY_NOP) i++;
}
//-------------------------------------------------------------------------------------------------
static void SPIInit(void)
{
	wiringPiSetup();
	pullUpDnControl(CHIP_POWER, PUD_OFF);
	pullUpDnControl(SPI_SCK, PUD_UP);
	pullUpDnControl(SPI_MISO, PUD_UP);
	pullUpDnControl(SPI_MOSI, PUD_UP);
	pullUpDnControl(SPI_CS, PUD_UP);
	pinMode(CHIP_POWER, OUTPUT);   Nop();
	digitalWrite(CHIP_POWER, LOW);
	delay(10);
	pinMode(SPI_SCK, OUTPUT);   Nop();
	pinMode(SPI_MISO, INPUT);   Nop();
	pinMode(SPI_MOSI, OUTPUT); 	Nop();
	pinMode(SPI_CS, OUTPUT);	Nop();
	Nop(); Nop(); Nop();
}
//-------------------------------------------------------------------------------------------------
static void SPI_beginTransaction(void)
{
	pinMode(SPI_CS, OUTPUT);	Nop();
	digitalWrite(SPI_CS, HIGH); Nop();
	pinMode(SPI_SCK, OUTPUT); 	Nop();
	pinMode(SPI_MISO, INPUT); 	Nop();
	pinMode(SPI_MOSI, OUTPUT); 	Nop();
	digitalWrite(SPI_SCK, LOW);	Nop();
	digitalWrite(SPI_CS, LOW);	Nop();
	Nop(); Nop(); Nop();
}
//-------------------------------------------------------------------------------------------------
static uint8_t SPI_transfer(uint8_t value)
{
	uint8_t result, mask;
	result = 0;
	for (mask = 0x80; mask != 0; mask = uint8_t(mask >> 1))
	{
		// Write and read 8 bit, starting form MSB
		if (value & mask)
			digitalWrite(SPI_MOSI, HIGH); 	// Write single 1
		else    digitalWrite(SPI_MOSI, LOW);	// Write single 0
		Nop();	Nop();	Nop();	Nop();
		digitalWrite(SPI_SCK, HIGH); 	Nop();	// Clock active on rising edge
		Nop(); Nop(); Nop();
		if (digitalRead(SPI_MISO) == HIGH)
			result = result | mask;
		Nop();	Nop();	Nop();	Nop();
		digitalWrite(SPI_SCK, LOW); // Clock to idle
		Nop();
	}
	return result;
}
//------------------------------------------------------------------------------
static void SPI_endTransaction(void)
{
	digitalWrite(SPI_CS, HIGH);
	Nop(); Nop(); Nop();
}
//------------------------------------------------------------------------------
void mfrc630_SPI_select(void)
{
	SPI_beginTransaction();
}
//------------------------------------------------------------------------------
void mfrc630_SPI_unselect(void)
{
	SPI_endTransaction();
}
//------------------------------------------------------------------------------
void mfrc630_SPI_transfer(const uint8_t* tx, uint8_t* rx, uint16_t len)
{
	uint16_t i;
	for (i = 0; i < len; i++)
		rx[i] = SPI_transfer(tx[i]);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Register interaction functions.
//------------------------------------------------------------------------------
uint8_t mfrc630_read_reg(uint8_t reg)
{
	uint8_t instruction_tx[2] = { 0 };
	uint8_t instruction_rx[2] = { 0 };
	instruction_tx[0] = uint8_t((reg << 1) | 0x01);
	mfrc630_SPI_select();
	mfrc630_SPI_transfer(instruction_tx, instruction_rx, 2);
	mfrc630_SPI_unselect();
	return instruction_rx[1];  // the second byte the returned value.
}
//------------------------------------------------------------------------------
void mfrc630_write_reg(uint8_t reg, uint8_t value)
{
	uint8_t instruction_tx[2] = { 0 };
	uint8_t discard[2] = { 0 };
	instruction_tx[0] = uint8_t((reg << 1) | 0x00);
	instruction_tx[1] = value;
	mfrc630_SPI_select();
	mfrc630_SPI_transfer(instruction_tx, discard, 2);
	mfrc630_SPI_unselect();
}
//------------------------------------------------------------------------------
/*void mfrc630_write_regs(uint8_t reg, const uint8_t* values, uint8_t len)
{
		uint8_t i, instruction_tx[50], discard[50];	//[len+1];
		if(len < 49)
		{
			  instruction_tx[0] = (reg << 1) | 0x00;
			  for (i=0 ; i < len; i++)
			  {
					  instruction_tx[i+1] = values[i];
			  }
			  mfrc630_SPI_select();
			  mfrc630_SPI_transfer(instruction_tx, discard, len+1);
			  mfrc630_SPI_unselect();
		}
		else
		{
				Nop();
		}
}*/
//------------------------------------------------------------------------------
void mfrc630_write_fifo(const uint8_t* data, uint16_t len)
{
	uint8_t write_instruction[1] = { (MFRC630_REG_FIFODATA << 1) | 0 };
	uint8_t discard[50];		//[len + 1];
	if (len < 49)
	{
		mfrc630_SPI_select();
		mfrc630_SPI_transfer(write_instruction, discard, 1);
		mfrc630_SPI_transfer(data, discard, len);
		mfrc630_SPI_unselect();
	}
	else
	{
		Nop();
	}
}
//------------------------------------------------------------------------------
void mfrc630_read_fifo(uint8_t* rx, uint16_t len)
{
	uint8_t read_instruction[] = { (MFRC630_REG_FIFODATA << 1) | 0x01, (MFRC630_REG_FIFODATA << 1) | 0x01 };
	uint8_t read_finish[] = { 0 };
	uint8_t discard[2];
	uint16_t i;
	// this is less than ideal, since we have to call the transfer method multiple times.
	mfrc630_SPI_select();
	mfrc630_SPI_transfer(read_instruction, discard, 1);
	for (i = 1; i < len; i++)
	{
		mfrc630_SPI_transfer(read_instruction, rx++, 1);
	}
	mfrc630_SPI_transfer(read_finish, rx++, 1);
	mfrc630_SPI_unselect();
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Command functions.
//------------------------------------------------------------------------------
/*void mfrc630_cmd_read_E2(uint16_t address, uint16_t length)
{
		uint8_t parameters[3] = {0};
		parameters[0] = (uint8_t) (address >> 8);
		parameters[1] = (uint8_t) (address & 0xFF);
		parameters[2] = length;
		mfrc630_flush_fifo();
		mfrc630_write_fifo(parameters, 3);
		mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_READE2);
}*/
//------------------------------------------------------------------------------
/*void mfrc630_cmd_load_reg(uint16_t address, uint8_t regaddr, uint16_t length)
{
		uint8_t parameters[4] = {0};
		parameters[0] = (uint8_t) (address >> 8);
		parameters[1] = (uint8_t) (address & 0xFF);
		parameters[2] = regaddr;
		parameters[3] = length;
		mfrc630_flush_fifo();
		mfrc630_write_fifo(parameters, 4);
		mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_LOADREG);
}*/
//------------------------------------------------------------------------------
/*void mfrc630_cmd_load_protocol(uint8_t rx, uint8_t tx)
{
		uint8_t parameters[2] = {0};
		parameters[0] = rx;
		parameters[1] = tx;
		mfrc630_flush_fifo();
		mfrc630_write_fifo(parameters, 2);
		mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_LOADPROTOCOL);
}*/
//------------------------------------------------------------------------------
void mfrc630_cmd_transceive(const uint8_t* data, uint16_t len)
{
	mfrc630_cmd_idle();
	mfrc630_flush_fifo();
	mfrc630_write_fifo(data, len);
	mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_TRANSCEIVE);
}
//------------------------------------------------------------------------------
void mfrc630_cmd_idle(void)
{
	mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_IDLE);
}
//------------------------------------------------------------------------------
/*void mfrc630_cmd_load_key_E2(uint8_t key_nr)
{
		uint8_t parameters[1] = {0};
		parameters[0] = key_nr;
		mfrc630_flush_fifo();
		mfrc630_write_fifo(parameters, 1);
		mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_LOADKEYE2);
}*/
//------------------------------------------------------------------------------
void mfrc630_cmd_auth(uint8_t key_type, uint8_t block_address, const uint8_t* card_uid)
{
	uint8_t parameters[6] = { 0 };
	parameters[0] = key_type;
	parameters[1] = block_address;
	parameters[2] = card_uid[0];
	parameters[3] = card_uid[1];
	parameters[4] = card_uid[2];
	parameters[5] = card_uid[3];
	mfrc630_cmd_idle();
	mfrc630_flush_fifo();
	mfrc630_write_fifo(parameters, 6);
	mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_MFAUTHENT);
}
//------------------------------------------------------------------------------
void mfrc630_cmd_load_key(const uint8_t* key)
{
	mfrc630_cmd_idle();
	mfrc630_flush_fifo();
	mfrc630_write_fifo(key, 6);
	mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_LOADKEY);
}
//------------------------------------------------------------------------------



// ---------------------------------------------------------------------------
// Utility functions.
// ---------------------------------------------------------------------------
void mfrc630_flush_fifo(void)
{
	mfrc630_write_reg(MFRC630_REG_FIFOCONTROL, 1 << 4);
}
//------------------------------------------------------------------------------
uint16_t mfrc630_fifo_length(void)
{
	// should do 512 byte fifo handling here.
	return mfrc630_read_reg(MFRC630_REG_FIFOLENGTH);
}
//------------------------------------------------------------------------------
void mfrc630_clear_irq0()
{
	mfrc630_write_reg(MFRC630_REG_IRQ0, uint8_t(~(1 << 7)));
}
//------------------------------------------------------------------------------
void mfrc630_clear_irq1()
{
	mfrc630_write_reg(MFRC630_REG_IRQ1, uint8_t(~(1 << 7)));
}
//------------------------------------------------------------------------------
uint8_t mfrc630_irq0()
{
	return mfrc630_read_reg(MFRC630_REG_IRQ0);
}
//------------------------------------------------------------------------------
uint8_t mfrc630_irq1()
{
	return mfrc630_read_reg(MFRC630_REG_IRQ1);
}
//------------------------------------------------------------------------------
/*uint8_t mfrc630_transfer_E2_page(uint8_t* dest, uint8_t page)
{
		uint8_t res;
		mfrc630_cmd_read_E2(page*64, 64);
		res = mfrc630_fifo_length();
		mfrc630_read_fifo(dest, 64);
		return res;
}*/
//------------------------------------------------------------------------------
void mfrc630_print_block(const uint8_t* data, uint16_t len)
{
	uint16_t i, j;
	for (i = 0; i < len; i++) {
		j = data[i];
		j++;
		MFRC630_PRINTF("%02X ", j--);
	}
}
//------------------------------------------------------------------------------



// ---------------------------------------------------------------------------
// Timer functions
// ---------------------------------------------------------------------------
/*void mfrc630_activate_timer(uint8_t timer, uint8_t active)
{
		mfrc630_write_reg(MFRC630_REG_TCONTROL, ((active << timer) << 4) | (1 << timer));
}*/
//------------------------------------------------------------------------------
void mfrc630_timer_set_control(uint8_t timer, uint8_t value)
{
	mfrc630_write_reg(uint8_t(MFRC630_REG_T0CONTROL + (5 * timer)), value);
}
//------------------------------------------------------------------------------
void mfrc630_timer_set_reload(uint8_t timer, uint16_t value)
{
	mfrc630_write_reg(uint8_t(MFRC630_REG_T0RELOADHI + (5 * timer)), uint8_t(value >> 8));
	mfrc630_write_reg(uint8_t(MFRC630_REG_T0RELOADLO + (5 * timer)), uint8_t(value & 0xFF));
}
//------------------------------------------------------------------------------
void mfrc630_timer_set_value(uint8_t timer, uint16_t value)
{
	mfrc630_write_reg(uint8_t(MFRC630_REG_T0COUNTERVALHI + (5 * timer)), uint8_t(value >> 8));
	mfrc630_write_reg(uint8_t(MFRC630_REG_T0COUNTERVALLO + (5 * timer)), uint8_t(value & 0xFF));
}
//------------------------------------------------------------------------------
/*uint16_t mfrc630_timer_get_value(uint8_t timer)
{
		uint16_t res = mfrc630_read_reg(MFRC630_REG_T0COUNTERVALHI + (5 * timer)) << 8;
		res += mfrc630_read_reg(MFRC630_REG_T0COUNTERVALLO + (5 * timer));
		return res;
}*/
//------------------------------------------------------------------------------



// ---------------------------------------------------------------------------
// From documentation
// ---------------------------------------------------------------------------
/*void mfrc630_AN11145_start_IQ_measurement()
{
		// Part-1, configurate LPCD Mode
		// Please remove any PICC from the HF of the reader.
		// "I" and the "Q" values read from reg 0x42 and 0x43
		// shall be used in part-2 "Detect PICC"
		//  reset CLRC663 and idle
		mfrc630_write_reg(0, 0x1F);
		// Should sleep here... for 50ms... can do without.
		mfrc630_write_reg(0, 0);
		// disable IRQ0, IRQ1 interrupt sources
		mfrc630_write_reg(0x06, 0x7F);
		mfrc630_write_reg(0x07, 0x7F);
		mfrc630_write_reg(0x08, 0x00);
		mfrc630_write_reg(0x09, 0x00);
		mfrc630_write_reg(0x02, 0xB0);  // Flush FIFO
		// LPCD_config
		mfrc630_write_reg(0x3F, 0xC0);  // Set Qmin register
		mfrc630_write_reg(0x40, 0xFF);  // Set Qmax register
		mfrc630_write_reg(0x41, 0xC0);  // Set Imin register
		mfrc630_write_reg(0x28, 0x89);  // set DrvMode register
		// Execute trimming procedure
		mfrc630_write_reg(0x1F, 0x00);  // Write default. T3 reload value Hi
		mfrc630_write_reg(0x20, 0x10);  // Write default. T3 reload value Lo
		mfrc630_write_reg(0x24, 0x00);  // Write min. T4 reload value Hi
		mfrc630_write_reg(0x25, 0x05);  // Write min. T4 reload value Lo
		mfrc630_write_reg(0x23, 0xF8);  // Config T4 for AutoLPCD&AutoRestart.Set AutoTrimm bit.Start T4.
		mfrc630_write_reg(0x43, 0x40);  // Clear LPCD result
		mfrc630_write_reg(0x38, 0x52);  // Set Rx_ADCmode bit
		mfrc630_write_reg(0x39, 0x03);  // Raise receiver gain to maximum
		mfrc630_write_reg(0x00, 0x01);  // Execute Rc663 command "Auto_T4" (Low power card detection and/or Auto trimming)
}*/
//------------------------------------------------------------------------------
/*void mfrc630_AN11145_stop_IQ_measurement()
{
		// Flush cmd and Fifo
		mfrc630_write_reg(0x00, 0x00);
		mfrc630_write_reg(0x02, 0xB0);
		mfrc630_write_reg(0x38, 0x12);  // Clear Rx_ADCmode bit
		//> ------------ I and Q Value for LPCD ----------------
		// mfrc630_read_reg(MFRC630_REG_LPCD_I_RESULT) & 0x3F
		// mfrc630_read_reg(MFRC630_REG_LPCD_Q_RESULT) & 0x3F
}*/
//------------------------------------------------------------------------------
/*void mfrc630_AN1102_recommended_registers_skip(uint8_t protocol, uint8_t skip)
{
		switch (protocol) {
				case MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER:
				{
						const uint8_t buf[] = MFRC630_RECOM_14443A_ID1_106;
						mfrc630_write_regs(MFRC630_REG_DRVMOD+skip, buf+skip, sizeof(buf)-skip);
				}
				break;
				case MFRC630_PROTO_ISO14443A_212_MILLER_BPSK:
				{
						const uint8_t buf[] = MFRC630_RECOM_14443A_ID1_212;
						mfrc630_write_regs(MFRC630_REG_DRVMOD+skip, buf+skip, sizeof(buf)-skip);
				}
				break;
				case MFRC630_PROTO_ISO14443A_424_MILLER_BPSK:
				{
						const uint8_t buf[] = MFRC630_RECOM_14443A_ID1_424;
						mfrc630_write_regs(MFRC630_REG_DRVMOD+skip, buf+skip, sizeof(buf)-skip);
				}
				break;
				case MFRC630_PROTO_ISO14443A_848_MILLER_BPSK:
				{
						const uint8_t buf[] = MFRC630_RECOM_14443A_ID1_848;
						mfrc630_write_regs(MFRC630_REG_DRVMOD+skip, buf+skip, sizeof(buf)-skip);
				}
				break;
		}
}*/
//------------------------------------------------------------------------------
/*void mfrc630_AN1102_recommended_registers(uint8_t protocol)
{
		mfrc630_AN1102_recommended_registers_skip(protocol, 0);
}*/
//------------------------------------------------------------------------------
/*void mfrc630_AN1102_recommended_registers_no_transmitter(uint8_t protocol)
{
		mfrc630_AN1102_recommended_registers_skip(protocol, 5);
}*/
//------------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// ISO 14443A
// ---------------------------------------------------------------------------
uint16_t mfrc630_iso14443a_REQA()
{
	return mfrc630_iso14443a_WUPA_REQA(MFRC630_ISO14443_CMD_REQA);
}
//------------------------------------------------------------------------------
/*uint16_t mfrc630_iso14443a_WUPA()
{
		return mfrc630_iso14443a_WUPA_REQA(MFRC630_ISO14443_CMD_WUPA);
}*/
//------------------------------------------------------------------------------
uint16_t mfrc630_iso14443a_WUPA_REQA(uint8_t instruction)
{
	uint8_t irq1_value, irq0, rx_len;
	uint8_t res_b[2];
	// configure a timeout timer.
	uint8_t timer_for_timeout = 0;
	uint8_t send_req[1];

	// ready the request.
	send_req[0] = instruction;
	mfrc630_cmd_idle();
	// mfrc630_AN1102_recommended_registers_no_transmitter(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
	mfrc630_flush_fifo();

	// Set register such that we sent 7 bits, set DataEn such that we can send
	// data.
	mfrc630_write_reg(MFRC630_REG_TXDATANUM, 7 | MFRC630_TXDATANUM_DATAEN);

	// disable the CRC registers.
	mfrc630_write_reg(MFRC630_REG_TXCRCPRESET, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_OFF);
	mfrc630_write_reg(MFRC630_REG_RXCRCCON, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_OFF);

	mfrc630_write_reg(MFRC630_REG_RXBITCTRL, 0);

	// clear interrupts
	mfrc630_clear_irq0();
	mfrc630_clear_irq1();

	// enable the global IRQ for Rx done and Errors.
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, MFRC630_IRQ0EN_RX_IRQEN | MFRC630_IRQ0EN_ERR_IRQEN);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, MFRC630_IRQ1EN_TIMER0_IRQEN);  // only trigger on timer for irq1



	// Set timer to 221 kHz clock, start at the end of Tx.
	mfrc630_timer_set_control(timer_for_timeout, MFRC630_TCONTROL_CLK_211KHZ | MFRC630_TCONTROL_START_TX_END);
	// Frame waiting time: FWT = (256 x 16/fc) x 2 FWI
	// FWI defaults to four... so that would mean wait for a maximum of ~ 5ms

	mfrc630_timer_set_reload(timer_for_timeout, 1000);  // 1000 ticks of 5 usec is 5 ms.
	mfrc630_timer_set_value(timer_for_timeout, 1000);

	// Go into send, then straight after in receive.
	mfrc630_cmd_transceive(send_req, 1);
	MFRC630_PRINTF("Sending REQA\n");
	// block until we are done
	irq1_value = 0;
	while (!(irq1_value & (1 << timer_for_timeout))) {
		irq1_value = mfrc630_irq1();
		if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {  // either ERR_IRQ or RX_IRQ
			break;  // stop polling irq1 and quit the timeout loop.
		}
	}
	MFRC630_PRINTF("After waiting for answer\n");
	mfrc630_cmd_idle();

	// if no Rx IRQ, or if there's an error somehow, return 0
	irq0 = mfrc630_irq0();
	if ((!(irq0 & MFRC630_IRQ0_RX_IRQ)) || (irq0 & MFRC630_IRQ0_ERR_IRQ)) {
		MFRC630_PRINTF("No RX, irq1: %hhx irq0: %hhx\n", irq1_value, irq0);
		return 0;
	}

	rx_len = uint8_t(mfrc630_fifo_length());

	MFRC630_PRINTF("rx_len: %hhd\n", rx_len);
	if (rx_len == 2) {  // ATQA should answer with 2 bytes.
		mfrc630_read_fifo(res_b, rx_len);

		MFRC630_PRINTF("ATQA answer: ");
		mfrc630_print_block(res_b, 2);
		MFRC630_PRINTF("\n");
		return uint16_t((res_b[0] << 8) + res_b[1]);
	}
	return 0;
}
//------------------------------------------------------------------------------
uint8_t mfrc630_iso14443a_select(uint8_t* uid, uint8_t* sak)
{
	uint8_t timer_for_timeout, cascade_level, message_length, collision_n;
	uint8_t irq0, error, coll, rxalign, rx_len, buf[5], collision_pos, choice_pos, selection;
	uint8_t rbx, bcc_val, bcc_calc, irq1_value, irq0_value, sak_len, sak_value, UIDn;

	mfrc630_cmd_idle();
	// mfrc630_AN1102_recommended_registers_no_transmitter(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
	mfrc630_flush_fifo();

	MFRC630_PRINTF("UID input: ");
	mfrc630_print_block(uid, 10);
	MFRC630_PRINTF("\n");

	MFRC630_PRINTF("\nStarting select\n");

	// we do not need atqa.
	// Bitshift to get uid_size; 0b00: single, 0b01: double, 0b10: triple, 0b11 RFU
	// uint8_t uid_size = (atqa & (0b11 << 6)) >> 6;
	// uint8_t bit_frame_collision = atqa & 0b11111;

	// enable the global IRQ for Rx done and Errors.
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, MFRC630_IRQ0EN_RX_IRQEN | MFRC630_IRQ0EN_ERR_IRQEN);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, MFRC630_IRQ1EN_TIMER0_IRQEN);  // only trigger on timer for irq1

	// configure a timeout timer, use timer 0.
	timer_for_timeout = 0;

	// Set timer to 221 kHz clock, start at the end of Tx.
	mfrc630_timer_set_control(timer_for_timeout, MFRC630_TCONTROL_CLK_211KHZ | MFRC630_TCONTROL_START_TX_END);
	// Frame waiting time: FWT = (256 x 16/fc) x 2 FWI
	// FWI defaults to four... so that would mean wait for a maximum of ~ 5ms

	mfrc630_timer_set_reload(timer_for_timeout, 1000);  // 1000 ticks of 5 usec is 5 ms.
	mfrc630_timer_set_value(timer_for_timeout, 1000);

	for (cascade_level = 1; cascade_level <= 3; cascade_level++) {
		uint8_t cmd = 0;
		uint8_t known_bits = 0;  // known bits of the UID at this level so far.
		uint8_t send_req[7] = { 0 };  // used as Tx buffer.
		uint8_t* uid_this_level = &(send_req[2]);
		// pointer to the UID so far, by placing this pointer in the send_req
		// array we prevent copying the UID continuously.
		switch (cascade_level) {
		case 1:
			cmd = MFRC630_ISO14443_CAS_LEVEL_1;
			break;
		case 2:
			cmd = MFRC630_ISO14443_CAS_LEVEL_2;
			break;
		case 3:
			cmd = MFRC630_ISO14443_CAS_LEVEL_3;
			break;
		}

		// disable CRC in anticipation of the anti collision protocol
		mfrc630_write_reg(MFRC630_REG_TXCRCPRESET, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_OFF);
		mfrc630_write_reg(MFRC630_REG_RXCRCCON, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_OFF);

		// max 32 loops of the collision loop.
		for (collision_n = 0; collision_n < 32; collision_n++) {
			MFRC630_PRINTF("\nCL: %hhd, coll loop: %hhd, kb %hhd long: ", cascade_level, collision_n, known_bits);
			mfrc630_print_block(uid_this_level, uint16_t((known_bits + 8 - 1) / 8));
			MFRC630_PRINTF("\n");

			// clear interrupts
			mfrc630_clear_irq0();
			mfrc630_clear_irq1();

			send_req[0] = cmd;
			send_req[1] = uint8_t(0x20 + known_bits);
			// send_req[2..] are filled with the UID via the uid_this_level pointer.

			// Only transmit the last 'x' bits of the current byte we are discovering
			// First limit the txdatanum, such that it limits the correct number of bits.
			mfrc630_write_reg(MFRC630_REG_TXDATANUM, uint8_t((known_bits % 8) | MFRC630_TXDATANUM_DATAEN));

			// ValuesAfterColl: If cleared, every received bit after a collision is
			// replaced by a zero. This function is needed for ISO/IEC14443 anticollision (0<<7).
			// We want to shift the bits with RxAlign
			rxalign = known_bits % 8;
			MFRC630_PRINTF("Setting rx align to: %hhd\n", rxalign);
			mfrc630_write_reg(MFRC630_REG_RXBITCTRL, uint8_t((0 << 7) | (rxalign << 4)));


			// then sent the send_req to the hardware,
			// (known_bits / 8) + 1): The ceiled number of bytes by known bits.
			// +2 for cmd and NVB.
			if ((known_bits % 8) == 0) {
				message_length = uint8_t(((known_bits / 8)) + 2);
			}
			else {
				message_length = uint8_t(((known_bits / 8) + 1) + 2);
			}

			MFRC630_PRINTF("Send:%hhd long: ", message_length);
			mfrc630_print_block(send_req, message_length);
			MFRC630_PRINTF("\n");

			mfrc630_cmd_transceive(send_req, message_length);


			// block until we are done
			irq1_value = 0;
			while (!(irq1_value & (1 << timer_for_timeout))) {
				irq1_value = mfrc630_irq1();
				// either ERR_IRQ or RX_IRQ or Timer
				if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {
					break;  // stop polling irq1 and quit the timeout loop.
				}
			}
			mfrc630_cmd_idle();

			// next up, we have to check what happened.
			irq0 = mfrc630_irq0();
			error = mfrc630_read_reg(MFRC630_REG_ERROR);
			coll = mfrc630_read_reg(MFRC630_REG_RXCOLL);
			MFRC630_PRINTF("irq0: %hhX\n", irq0);
			MFRC630_PRINTF("error: %hhX\n", error);
			collision_pos = 0;
			if (irq0 & MFRC630_IRQ0_ERR_IRQ) {  // some error occured.
				// Check what kind of error.
				// error = mfrc630_read_reg(MFRC630_REG_ERROR);
				if (error & MFRC630_ERROR_COLLDET) {
					// A collision was detected...
					if (coll & (1 << 7)) {
						collision_pos = uint8_t(coll & (~(1 << 7)));
						MFRC630_PRINTF("Collision at %hhX\n", collision_pos);
						// This be a true collision... we have to select either the address
						// with 1 at this position or with zero
						// ISO spec says typically a 1 is added, that would mean:
						// uint8_t selection = 1;

						// However, it makes sense to allow some kind of user input for this, so we use the
						// current value of uid at this position, first index right byte, then shift such
						// that it is in the rightmost position, ten select the last bit only.
						// We cannot compensate for the addition of the cascade tag, so this really
						// only works for the first cascade level, since we only know whether we had
						// a cascade level at the end when the SAK was received.
						choice_pos = uint8_t(known_bits + collision_pos);
						selection = uint8_t((uid[((choice_pos + (cascade_level - 1) * 3) / 8)] >> ((choice_pos) % 8)) & 1);


						// We just OR this into the UID at the right position, later we
						// OR the UID up to this point into uid_this_level.
						uid_this_level[((choice_pos) / 8)] = uint8_t(uid_this_level[(choice_pos) / 8] | (selection << ((choice_pos) % 8)));
						known_bits++;  // add the bit we just decided.

						MFRC630_PRINTF("uid_this_level now kb %hhd long: ", known_bits);
						mfrc630_print_block(uid_this_level, 10);
						MFRC630_PRINTF("\n");

					}
					else {
						// Datasheet of mfrc630:
						// bit 7 (CollPosValid) not set:
						// Otherwise no collision is detected or
						// the position of the collision is out of the range of bits CollPos.
						MFRC630_PRINTF("Collision but no valid collpos.\n");
						collision_pos = uint8_t(0x20 - known_bits);
					}
				}
				else {
					// Can this ever occur?
					collision_pos = uint8_t(0x20 - known_bits);
					MFRC630_PRINTF("No collision, error was: %hhx, setting collision_pos to: %hhx\n", error, collision_pos);
				}
			}
			else if (irq0 & MFRC630_IRQ0_RX_IRQ) {
				// we got data, and no collisions, that means all is well.
				collision_pos = uint8_t(0x20 - known_bits);
				MFRC630_PRINTF("Got data, no collision, setting to: %hhx\n", collision_pos);
			}
			else {
				// We have no error, nor received an RX. No response, no card?
				return 0;
			}
			MFRC630_PRINTF("collision_pos: %hhX\n", collision_pos);

			// read the UID Cln so far from the buffer.
			rx_len = uint8_t(mfrc630_fifo_length());

			mfrc630_read_fifo(buf, rx_len < 5 ? rx_len : 5);

			MFRC630_PRINTF("Fifo %hhd long: ", rx_len);
			mfrc630_print_block(buf, rx_len);
			MFRC630_PRINTF("\n");

			MFRC630_PRINTF("uid_this_level kb %hhd long: ", known_bits);
			mfrc630_print_block(uid_this_level, uint16_t((known_bits + 8 - 1) / 8));
			MFRC630_PRINTF("\n");
			// move the buffer into the uid at this level, but OR the result such that
			// we do not lose the bit we just set if we have a collision.
			for (rbx = 0; (rbx < rx_len); rbx++) {
				uid_this_level[(known_bits / 8) + rbx] |= buf[rbx];
			}
			known_bits = uint8_t(known_bits + collision_pos);
			MFRC630_PRINTF("known_bits: %hhX\n", known_bits);

			if ((known_bits >= 32)) {
				MFRC630_PRINTF("exit collision loop: uid_this_level kb %hhd long: ", known_bits);
				mfrc630_print_block(uid_this_level, 10);
				MFRC630_PRINTF("\n");

				break;  // done with collision loop
			}
		}  // end collission loop

		// check if the BCC matches
		bcc_val = uid_this_level[4];  // always at position 4, either with CT UID[0-2] or UID[0-3] in front.
		bcc_calc = uid_this_level[0] ^ uid_this_level[1] ^ uid_this_level[2] ^ uid_this_level[3];
		if (bcc_val != bcc_calc) {
			MFRC630_PRINTF("Something went wrong, BCC does not match.\n");
			return 0;
		}

		// clear interrupts
		mfrc630_clear_irq0();
		mfrc630_clear_irq1();

		send_req[0] = cmd;
		send_req[1] = 0x70;
		// send_req[2,3,4,5] // contain the CT, UID[0-2] or UID[0-3]
		send_req[6] = bcc_calc;
		message_length = 7;

		// Ok, almost done now, we reenable the CRC's
		mfrc630_write_reg(MFRC630_REG_TXCRCPRESET, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_ON);
		mfrc630_write_reg(MFRC630_REG_RXCRCCON, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_ON);

		// reset the Tx and Rx registers (disable alignment, transmit full bytes)
		mfrc630_write_reg(MFRC630_REG_TXDATANUM, uint8_t((known_bits % 8) | MFRC630_TXDATANUM_DATAEN));
		rxalign = 0;
		mfrc630_write_reg(MFRC630_REG_RXBITCTRL, uint8_t((0 << 7) | (rxalign << 4)));

		// actually send it!
		mfrc630_cmd_transceive(send_req, message_length);
		MFRC630_PRINTF("send_req %hhd long: ", message_length);
		mfrc630_print_block(send_req, message_length);
		MFRC630_PRINTF("\n");

		// Block until we are done...
		irq1_value = 0;
		while (!(irq1_value & (1 << timer_for_timeout))) {
			irq1_value = mfrc630_irq1();
			if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {  // either ERR_IRQ or RX_IRQ
				break;  // stop polling irq1 and quit the timeout loop.
			}
		}
		mfrc630_cmd_idle();

		// Check the source of exiting the loop.
		irq0_value = mfrc630_irq0();
		if (irq0_value & MFRC630_IRQ0_ERR_IRQ) {
			// Check what kind of error.
			error = mfrc630_read_reg(MFRC630_REG_ERROR);
			if (error & MFRC630_ERROR_COLLDET) {
				// a collision was detected with NVB=0x70, should never happen.
				return 0;
			}
		}

		// Read the sak answer from the fifo.
		sak_len = uint8_t(mfrc630_fifo_length());
		if (sak_len != 1) {
			return 0;
		}
		mfrc630_read_fifo(&sak_value, sak_len);

		MFRC630_PRINTF("SAK answer: ");
		mfrc630_print_block(&sak_value, 1);
		MFRC630_PRINTF("\n");

		if (sak_value & (1 << 2)) {
			// UID not yet complete, continue with next cascade.
			// This also means the 0'th byte of the UID in this level was CT, so we
			// have to shift all bytes when moving to uid from uid_this_level.
			for (UIDn = 0; UIDn < 3; UIDn++) {
				// uid_this_level[UIDn] = uid_this_level[UIDn + 1];
				uid[(cascade_level - 1) * 3 + UIDn] = uid_this_level[UIDn + 1];
			}
		}
		else {
			// Done according so SAK!
			// Add the bytes at this level to the UID.
			for (UIDn = 0; UIDn < 4; UIDn++) {
				uid[(cascade_level - 1) * 3 + UIDn] = uid_this_level[UIDn];
			}

			*sak = sak_value;
			// Finally, return the length of the UID that's now at the uid pointer.
			return uint8_t(cascade_level * 3 + 1);
		}

		MFRC630_PRINTF("Exit cascade %hhd long: ", cascade_level);
		mfrc630_print_block(uid, 10);
		MFRC630_PRINTF("\n");
	}  // cascade loop
	return 0;  // getting an UID failed.
}
//------------------------------------------------------------------------------



// ---------------------------------------------------------------------------
// MIFARE
// ---------------------------------------------------------------------------
uint8_t mfrc630_MF_auth(const uint8_t* uid, uint8_t key_type, uint8_t block)
{
	uint8_t timer_for_timeout, irq1_value, status;
	// Enable the right interrupts.

	// configure a timeout timer.
	timer_for_timeout = 0;  // should match the enabled interupt.

	// According to datashet Interrupt on idle and timer with MFAUTHENT, but lets
	// include ERROR as well.
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, MFRC630_IRQ0EN_IDLE_IRQEN | MFRC630_IRQ0EN_ERR_IRQEN);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, MFRC630_IRQ1EN_TIMER0_IRQEN);  // only trigger on timer for irq1

	// Set timer to 221 kHz clock, start at the end of Tx.
	mfrc630_timer_set_control(timer_for_timeout, MFRC630_TCONTROL_CLK_211KHZ | MFRC630_TCONTROL_START_TX_END);
	// Frame waiting time: FWT = (256 x 16/fc) x 2 FWI
	// FWI defaults to four... so that would mean wait for a maximum of ~ 5ms

	mfrc630_timer_set_reload(timer_for_timeout, 2000);  // 2000 ticks of 5 usec is 10 ms.
	mfrc630_timer_set_value(timer_for_timeout, 2000);

	irq1_value = 0;

	mfrc630_clear_irq0();  // clear irq0
	mfrc630_clear_irq1();  // clear irq1

	// start the authentication procedure.
	mfrc630_cmd_auth(key_type, block, uid);

	// block until we are done
	while (!(irq1_value & (1 << timer_for_timeout)))
	{
		irq1_value = mfrc630_irq1();
		if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {
			break;  // stop polling irq1 and quit the timeout loop.
		}
	}

	if (irq1_value & (1 << timer_for_timeout)) {
		// this indicates a timeout
		return 0;  // we have no authentication
	}

	// status is always valid, it is set to 0 in case of authentication failure.
	status = mfrc630_read_reg(MFRC630_REG_STATUS);
	return (status & MFRC630_STATUS_CRYPTO1_ON);
}
//------------------------------------------------------------------------------
uint8_t mfrc630_MF_read_block(uint8_t block_address, uint8_t* dest)
{
	uint8_t send_req[2], timer_for_timeout, irq0_value, irq1_value, buffer_length, rx_len;
	mfrc630_flush_fifo();

	mfrc630_write_reg(MFRC630_REG_TXCRCPRESET, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_ON);
	mfrc630_write_reg(MFRC630_REG_RXCRCCON, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_ON);

	send_req[0] = MFRC630_MF_CMD_READ;
	send_req[1] = block_address;

	// configure a timeout timer.
	timer_for_timeout = 0;  // should match the enabled interupt.

	// enable the global IRQ for idle, errors and timer.
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, MFRC630_IRQ0EN_IDLE_IRQEN | MFRC630_IRQ0EN_ERR_IRQEN);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, MFRC630_IRQ1EN_TIMER0_IRQEN);


	// Set timer to 221 kHz clock, start at the end of Tx.
	mfrc630_timer_set_control(timer_for_timeout, MFRC630_TCONTROL_CLK_211KHZ | MFRC630_TCONTROL_START_TX_END);
	// Frame waiting time: FWT = (256 x 16/fc) x 2 FWI
	// FWI defaults to four... so that would mean wait for a maximum of ~ 5ms
	mfrc630_timer_set_reload(timer_for_timeout, 2000);  // 2000 ticks of 5 usec is 10 ms.
	mfrc630_timer_set_value(timer_for_timeout, 2000);

	irq1_value = 0;
	irq0_value = 0;

	mfrc630_clear_irq0();  // clear irq0
	mfrc630_clear_irq1();  // clear irq1

	// Go into send, then straight after in receive.
	mfrc630_cmd_transceive(send_req, 2);

	// block until we are done
	while (!(irq1_value & (1 << timer_for_timeout))) {
		irq1_value = mfrc630_irq1();
		if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {
			break;  // stop polling irq1 and quit the timeout loop.
		}
	}
	mfrc630_cmd_idle();

	if (irq1_value & (1 << timer_for_timeout)) {
		// this indicates a timeout
		return 0;
	}

	irq0_value = mfrc630_irq0();
	if (irq0_value & MFRC630_IRQ0_ERR_IRQ) {
		// some error
		return 0;
	}

	// all seems to be well...
	buffer_length = uint8_t(mfrc630_fifo_length());
	rx_len = (buffer_length <= 16) ? buffer_length : 16;
	mfrc630_read_fifo(dest, rx_len);
	return rx_len;
}
//------------------------------------------------------------------------------
// The read and write block functions share a lot of code, the parts they have in common could perhaps be extracted to make it more readable.
uint8_t mfrc630_MF_write_block(uint8_t block_address, const uint8_t* source)
{
	uint8_t res, send_req[2], irq1_value, irq0_value, buffer_length, timer_for_timeout;
	mfrc630_flush_fifo();

	// set appropriate CRC registers, only for Tx
	mfrc630_write_reg(MFRC630_REG_TXCRCPRESET, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_ON);
	mfrc630_write_reg(MFRC630_REG_RXCRCCON, MFRC630_RECOM_14443A_CRC | MFRC630_CRC_OFF);
	// configure a timeout timer.
	timer_for_timeout = 0;  // should match the enabled interupt.

	// enable the global IRQ for idle, errors and timer.
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, MFRC630_IRQ0EN_IDLE_IRQEN | MFRC630_IRQ0EN_ERR_IRQEN);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, MFRC630_IRQ1EN_TIMER0_IRQEN);

	// Set timer to 221 kHz clock, start at the end of Tx.
	mfrc630_timer_set_control(timer_for_timeout, MFRC630_TCONTROL_CLK_211KHZ | MFRC630_TCONTROL_START_TX_END);
	// Frame waiting time: FWT = (256 x 16/fc) x 2 FWI
	// FWI defaults to four... so that would mean wait for a maximum of ~ 5ms
	mfrc630_timer_set_reload(timer_for_timeout, 2000);  // 2000 ticks of 5 usec is 10 ms.
	mfrc630_timer_set_value(timer_for_timeout, 2000);

	irq1_value = 0;
	irq0_value = 0;
	send_req[0] = MFRC630_MF_CMD_WRITE;
	send_req[1] = block_address;

	mfrc630_clear_irq0();  // clear irq0
	mfrc630_clear_irq1();  // clear irq1

	// Go into send, then straight after in receive.
	mfrc630_cmd_transceive(send_req, 2);

	// block until we are done
	while (!(irq1_value & (1 << timer_for_timeout))) {
		irq1_value = mfrc630_irq1();
		if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {
			break;  // stop polling irq1 and quit the timeout loop.
		}
	}
	mfrc630_cmd_idle();

	// check if the first stage was successful:
	if (irq1_value & (1 << timer_for_timeout)) {
		// this indicates a timeout
		return 0;
	}
	irq0_value = mfrc630_irq0();
	if (irq0_value & MFRC630_IRQ0_ERR_IRQ) {
		// some error
		return 0;
	}
	buffer_length = uint8_t(mfrc630_fifo_length());
	if (buffer_length != 1) {
		return 0;
	}
	mfrc630_read_fifo(&res, 1);
	if (res != MFRC630_MF_ACK) {
		return 0;
	}

	mfrc630_clear_irq0();  // clear irq0
	mfrc630_clear_irq1();  // clear irq1

	// go for the second stage.
	mfrc630_cmd_transceive(source, 16);

	// block until we are done
	while (!(irq1_value & (1 << timer_for_timeout))) {
		irq1_value = mfrc630_irq1();
		if (irq1_value & MFRC630_IRQ1_GLOBAL_IRQ) {
			break;  // stop polling irq1 and quit the timeout loop.
		}
	}

	mfrc630_cmd_idle();

	if (irq1_value & (1 << timer_for_timeout)) {
		// this indicates a timeout
		return 0;
	}
	irq0_value = mfrc630_irq0();
	if (irq0_value & MFRC630_IRQ0_ERR_IRQ) {
		// some error
		return 0;
	}

	buffer_length = uint8_t(mfrc630_fifo_length());
	if (buffer_length != 1) {
		return 0;
	}
	mfrc630_read_fifo(&res, 1);
	if (res == MFRC630_MF_ACK) {
		return 16;  // second stage was responded with ack! Write successful.
	}

	return 0;
}
//------------------------------------------------------------------------------
void mfrc630_MF_deauth()
{
	mfrc630_write_reg(MFRC630_REG_STATUS, 0);
}
//------------------------------------------------------------------------------
/*void mfrc630_init()
{
/// più efficiente come portata, ma non funziona scrittura su clr6633

		// Set the registers of the MFRC630 into the default.
	mfrc630_AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);

	// This are register required for my development platform, you probably have to change (or uncomment) them.
	mfrc630_write_reg(MFRC630_REG_DRVMOD, 0x8E);    // Inverts transmitter 2 at TX2 pin, pullup
	mfrc630_write_reg(MFRC630_REG_TXAMP, 0x15);     // residual carrier 78%, modulation index 12.4%
	mfrc630_write_reg(MFRC630_REG_DRVCON, 0x11);    // 1 carrier clocks, TxEnvelope
	mfrc630_write_reg(MFRC630_REG_TXL, 0x06);       // sets the expected Tx load current.
}*/
//------------------------------------------------------------------------------
void PCD_ReInit(void)
{
	printf("Transponder Reinizializzato\r\n");
	// 120Ms
	if (mfrc630_iso_14443A_init() == TRUE)
		printf("   Transponder OK\r\n");
	mfrc630_MF_deauth();
}
//-------------------------------------------------------------------------------------------------
uint8_t mfrc630_iso_14443A_init()
{
	/// meno efficiente, ma funziona bene sia su clr630 che clr6633

	uint8_t count, version;

	SPIInit();

	digitalWrite(CHIP_POWER, LOW);
	delay(100);
	digitalWrite(CHIP_POWER, HIGH);
	delay(10);

	version = mfrc630_read_reg(MFRC630_REG_VERSION);
	if ((version & 0xF0) != 0x10)    // se non è un modulo MFRC630 errore
		return 0;

	//> Configure Timers
	mfrc630_write_reg(MFRC630_REG_T0CONTROL, 0x98);           //Starts at the end of Tx. Stops after Rx of first data. Auto-reloaded. 13.56 MHz input clock.
	mfrc630_write_reg(MFRC630_REG_T1CONTROL, 0x92);          //Starts at the end of Tx. Stops after Rx of first data. Input clock - cascaded with Timer-0.
	mfrc630_write_reg(MFRC630_REG_T2CONTROL, 0x20);          //Set Timer-2, T2Control_Reg:  Timer used for LFO trimming
	mfrc630_write_reg(MFRC630_REG_T2RELOADHI, 0x03);         //Set Timer-2 reload value (T2ReloadHi_Reg and T2ReloadLo_Reg)
	mfrc630_write_reg(MFRC630_REG_T2RELOADLO, 0xFF);         //
	mfrc630_write_reg(MFRC630_REG_T3CONTROL, 0x00);          // Not started automatically. Not reloaded. Input clock 13.56 MHz
	mfrc630_write_reg(MFRC630_REG_FIFOCONTROL, 0x10);

	mfrc630_write_reg(MFRC630_REG_WATERLEVEL, 0xFE);        //Set WaterLevel =(FIFO length -1),cause fifo length has been set to 255=0xff,so water level is oxfe
	mfrc630_write_reg(MFRC630_REG_RXBITCTRL, 0x80);         //RxBitCtrl_Reg(0x0c)  Received bit after collision are replaced with 1.
	mfrc630_write_reg(MFRC630_REG_DRVMOD, 0x80);            //DrvMod reg(0x28), Tx2Inv=1,Inverts transmitter 1 at TX1 pin
	mfrc630_write_reg(MFRC630_REG_TXAMP, 0x00);             // TxAmp_Reg(0x29),output amplitude  0: TVDD -100 mV(maxmum)
	mfrc630_write_reg(MFRC630_REG_DRVCON, 0x01);            // TxCon register (address 2Ah),TxEnvelope
	mfrc630_write_reg(MFRC630_REG_TXL, 0x05);               //
	mfrc630_write_reg(MFRC630_REG_RXSOFD, 0x00);            //
	mfrc630_write_reg(MFRC630_REG_RCV, 0x12);               //
	//> =============================================
	//>  LoadProtocol( bTxProtocol=0, bRxProtocol=0)
	//> =============================================
	mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_IDLE);           // Terminate any running command.
	mfrc630_write_reg(MFRC630_REG_FIFOCONTROL, 0xB0);       // Flush_FiFo,low alert


	mfrc630_write_reg(MFRC630_REG_IRQ0, 0x7F);             // Clear all IRQ 0,1 flags
	mfrc630_write_reg(MFRC630_REG_IRQ1, 0x7F);             //
	//> Write in Fifo: Tx and Rx protocol numbers(0,0)
	mfrc630_write_reg(MFRC630_REG_FIFODATA, 0x00);         //
	mfrc630_write_reg(MFRC630_REG_FIFODATA, 0x00);         //
	mfrc630_write_reg(MFRC630_REG_COMMAND, MFRC630_CMD_LOADPROTOCOL);    // Start RC663 command "Load Protocol"=0x0d

	//mfrc630_irq_wait(0x10, 0x00);              // Wait for idle interrupt meaning the command has finished
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, 0x10);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, 0x00);

	count = 0;
	while (!(mfrc630_read_reg(MFRC630_REG_IRQ1) & 0x40))    //Wait untill global interrupt set
	{
		delay(10);
		count++;
		if (count > 100)
			return 0;       // dopo timeou essce comunque
	}
	mfrc630_write_reg(MFRC630_REG_IRQ0EN, 0x00);
	mfrc630_write_reg(MFRC630_REG_IRQ1EN, 0x00);

	mfrc630_write_reg(MFRC630_REG_FIFOCONTROL, 0xB0);       // Flush_FiFo

	// Apply RegisterSet
	//
	//> Configure CRC-16 calculation, preset value(0x6363) for Tx&Rx

	mfrc630_write_reg(MFRC630_REG_TXCRCPRESET, 0x18);           //means preset value is 6363,and uses CRC 16,but CRC is not automaticlly apended to the data
	mfrc630_write_reg(MFRC630_REG_RXCRCCON, 0x18);           //


	mfrc630_write_reg(MFRC630_REG_TXDATANUM, 0x08);             //
	mfrc630_write_reg(MFRC630_REG_TXMODWIDTH, 0x20);            // Length of the pulse modulation in carrier clks+1
	mfrc630_write_reg(MFRC630_REG_TXSYM10BURSTLEN, 0x00);       // Symbol 1 and 0 burst lengths = 8 bits.
	mfrc630_write_reg(MFRC630_REG_FRAMECON, 0xCF);             // Start symbol=Symbol2, Stop symbol=Symbol3
	mfrc630_write_reg(MFRC630_REG_RXCTRL, 0x04);               // Set Rx Baudrate 106 kBaud

	mfrc630_write_reg(MFRC630_REG_RXTHRESHOLD, 0x32);          // Set min-levels for Rx and phase shift
	mfrc630_write_reg(MFRC630_REG_RXANA, 0x00);
	mfrc630_write_reg(MFRC630_REG_RXWAIT, 0x90);             // Set Rx waiting time
	mfrc630_write_reg(MFRC630_REG_TXWAITCTRL, 0xC0);
	mfrc630_write_reg(MFRC630_REG_TXWAITLO, 0x0B);
	mfrc630_write_reg(MFRC630_REG_T0RELOADHI, 0x08);         // Set Timeout. Write T0,T1 reload values(hi,Low)
	mfrc630_write_reg(MFRC630_REG_T0RELOADLO, 0xD8);
	mfrc630_write_reg(MFRC630_REG_T1RELOADHI, 0x00);
	mfrc630_write_reg(MFRC630_REG_T1RELOADLO, 0x00);

	mfrc630_write_reg(MFRC630_REG_DRVMOD, 0x8E);  //0x81);               // Write DrvMod register

	//> MIFARE Crypto1 state is further disabled.
	mfrc630_write_reg(MFRC630_REG_STATUS, 0x00);

	return TRUE;
}
//------------------------------------------------------------------------------
uint8_t mfrc630_MF_ready(Uid* uid)
{
	// restituisce 1 se tessera presente e uid, altrimenti 0
	uint16_t atqa;
	atqa = mfrc630_iso14443a_REQA();
	if (atqa == 0)
		atqa = mfrc630_iso14443a_REQA();        // ritento perchè dopo mfrc630_MF_deauth prima lettura a 0
	if (atqa != 0)
	{
		// Select the card and discover its uid.
		uid->size = mfrc630_iso14443a_select(uid->uidByte, &uid->sak);
		if (uid->size != 0)
		{
			return 1;
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
/*void mfrc630_MF_example_dump()
{
		uint16_t atqa = mfrc630_iso14443a_REQA();
		if (atqa != 0) {  // Are there any cards that answered?
		  uint8_t sak;
		  uint8_t uid[10] = {0};  // uids are maximum of 10 bytes long.
		  // Use the manufacturer default key...
		  uint8_t FFkey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

		  // Select the card and discover its uid.
		  uint8_t uid_len = mfrc630_iso14443a_select(uid, &sak);

		  if (uid_len != 0) {  // did we get an UID?
			MFRC630_PRINTF("UID of %hhd bytes (SAK:0x%hhX): ", uid_len, sak);
			mfrc630_print_block(uid, uid_len);
			MFRC630_PRINTF("\n");

			mfrc630_cmd_load_key(FFkey);  // load into the key buffer

			// Try to athenticate block 0.
			if (mfrc630_MF_auth(uid, MFRC630_MF_AUTH_KEY_A, 0)) {
			  uint8_t readbuf[16] = {0};
			  uint8_t len;
			  uint8_t b;
			  MFRC630_PRINTF("Yay! We are authenticated!\n");

			  // Attempt to read the first 4 blocks.

			  for (b=0; b < 4 ; b++) {
				len = mfrc630_MF_read_block(b, readbuf);
				MFRC630_PRINTF("Read block 0x%hhX: ", b);
				mfrc630_print_block(readbuf, len);
				MFRC630_PRINTF("\n");
			  }
			  mfrc630_MF_deauth();  // be sure to call this after an authentication!
			} else {
			  MFRC630_PRINTF("Could not authenticate :(\n");
			}
		  } else {
			MFRC630_PRINTF("Could not determine UID, perhaps some cards don't play");
			MFRC630_PRINTF(" well with the other cards? Or too many collisions?\n");
		  }
		} else {
		  MFRC630_PRINTF("No answer to REQA, no cards?\n");
		}
}*/
//------------------------------------------------------------------------------
#endif
