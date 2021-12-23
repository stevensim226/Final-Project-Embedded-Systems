#include <asf.h>
#include <stdio.h>
#include <adc_sensors/adc_sensors.h>
#include <string.h>
#include <ioport.h>
#include <board.h>

#define USART_SERIAL_EXAMPLE             &USARTC0
#define USART_SERIAL_EXAMPLE_BAUDRATE    9600
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

static char reads[100];
static char strbuf[128];

void setUpSerial()
{
	// Baud rate selection
	// BSEL = (2000000 / (2^0 * 16*9600) -1 = 12.0208... ~ 12 -> BSCALE = 0
	// FBAUD = ( (2000000)/(2^0*16(12+1)) = 9615.384 -> mendekati lah ya
	
	USARTC0_BAUDCTRLB = 0; //memastikan BSCALE = 0
	USARTC0_BAUDCTRLA = 0x0C; // 12
	
	USARTC0_BAUDCTRLB = 0; //Just to be sure that BSCALE is 0
	USARTC0_BAUDCTRLA = 0xCF; // 207
	
	//Disable interrupts, just for safety
	USARTC0_CTRLA = 0;
	//8 data bits, no parity and 1 stop bit
	USARTC0_CTRLC = USART_CHSIZE_8BIT_gc;
	
	//Enable receive and transmit
	USARTC0_CTRLB = USART_TXEN_bm | USART_RXEN_bm;
}

void sendChar(char c) {
	while( !(USARTC0_STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty
	USARTC0_DATA = c;
}

void sendString(char *text) {
	while(*text) {
		sendChar(*text++);
		//usart_putchar(USART_SERIAL_EXAMPLE, *text++);
		delay_ms(1);
	}
}

char receiveChar() {
	while (!(USARTC0_STATUS & USART_RXCIF_bm)); //Wait until receive finish
	return USARTC0_DATA;
}

void receiveString() {
	int i = 0;
	while(1){
		char inp = receiveChar();
		//char inp = usart_getchar(USART_SERIAL_EXAMPLE);
		if(inp=='\n') break;
		else reads[i++] = inp;
		delay_ms(1);
	}
	reads[i] = '\0';
}


/* Define semaphore */
static char strbuf[128];

int8_t readTemp (void)
{
	ntc_measure();												// Mengambil data dari pengukuran suhu oleh NTC temperature sensor
	while(!ntc_data_is_ready());								// Menunggu data sampai siap untuk ditampilkan
	int8_t temperature = ntc_get_temperature();	// Mengambil hasil olah data dalam Celcius
	return temperature;
}

void writeLCD(char text[128], int x, int y) {
	gfx_mono_draw_string(text,x, y, &sysfont);
}

int main (void)
{
	board_init();
	sysclk_init();

	//init lcd
	gfx_mono_init();
	//set background lcd on
	gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);
	adc_sensors_init();
	
	// Inisialisasi interrupt vector
	pmic_init();
	cpu_irq_enable();
	
	PORTC_OUTSET = PIN3_bm; // PC3 as TX
	PORTC_DIRSET = PIN3_bm; //TX pin as output
	PORTC_OUTCLR = PIN2_bm; //PC2 as RX
	PORTC_DIRCLR = PIN2_bm; //RX pin as input	
	setUpSerial();
	
		
	static usart_rs232_options_t USART_SERIAL_OPTIONS = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	};
		
	usart_init_rs232(USART_SERIAL_EXAMPLE, &USART_SERIAL_OPTIONS);
	
	ioport_set_pin_dir(J2_PIN0, IOPORT_DIR_OUTPUT);

	while (1) {
		snprintf(strbuf, sizeof(strbuf), "Tempr : %3d",readTemp());
		writeLCD(strbuf,0,0);
		delay_ms(300);
		writeLCD("TEMPR",0,0);
		delay_ms(300);
		
		receiveString();
		//writeLCD(reads,0,8); // DEBUG: Print read value from Arduino
		
		// Detect if fire signal is received, 2 means fire
		if (strstr(reads, "2") != NULL) {
			writeLCD("Fire detected!  ",0,24);
		} else {
			writeLCD("Normal situation",0,24);
		}
		delay_ms(300);
	}
}