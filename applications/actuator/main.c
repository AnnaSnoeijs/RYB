#include <libpynq.h>
#include <stdio.h>

#define CRYING_ADDRESS    0x10
#define HEARTBEAT_ADDRESS 0x20
#define ALGORITHM_ADDRESS 0x30
#define ACTUATOR_ADDRESS  0x40

#define AMPLITUDE_MAX 5
#define FREQUENCY_MAX 5

#define BACKGROUND_COLOR RGB_BLACK
#define TEXT_COLOR RGB_GREEN

#define FONTSIZE 16
#define FONTWIDTH 8

#define LCD_TEXT_X 72
#define LCD_TEXT_Y 104

void printInt(
	display_t *display,
	FontxFile *fx16G,
	int x,
	int y,
	const char *string,
	int n
){
	char str[32]="";
	int size = sprintf(str, string, n);
	for(int i = 0; i < size; i++){
		displayDrawFillRect(
	        	display,
	        	x + i * FONTWIDTH,
	        	y,
	        	x + (i+1) * FONTWIDTH,
	        	y + FONTSIZE - 1,
	        	BACKGROUND_COLOR
		);
		displayDrawChar(
			display,
			fx16G,
	        	x + i * FONTWIDTH,
	        	y + FONTSIZE,
			str[i],
			TEXT_COLOR
		);
	}
}

void initPrintData(
	display_t *display,
	FontxFile *fx16G
){
	displayFillScreen(display, BACKGROUND_COLOR);
	displayDrawString(
		display,
		fx16G,
		LCD_TEXT_X,
		LCD_TEXT_Y + FONTSIZE,
		(uint8_t *)"Amplitude:",
		TEXT_COLOR
	);
	displayDrawString(
		display,
		fx16G,
		LCD_TEXT_X,
		LCD_TEXT_Y + 2 * FONTSIZE,
		(uint8_t *)"Frequency:",
		TEXT_COLOR
	);
}

void printData(
	display_t *display,
	FontxFile *fx16G,
	uint8_t  command
){
	printInt(
		display,
		fx16G,
		LCD_TEXT_X + 10 * FONTWIDTH,
		LCD_TEXT_Y,
		"%d",
		command >> 4
	);
	printInt(
		display,
		fx16G,
		LCD_TEXT_X + 10 * FONTWIDTH,
		LCD_TEXT_Y + FONTSIZE,
		"%d",
		command & 0x0f
	);
}

int main(){
	// It ain't a bo'o o' wo'er init?
	// Ra'er sunny wea'er init?
	pynq_init();

	// set up screen font
	uint8_t
		buffer_fx16G[FontxGlyphBufSize],
		fontWidth_fx16G,fontHeight_fx16G;
	FontxFile fx16G[2];
	InitFontx(fx16G, "/boot/ILGH16XB.FNT", "");
	GetFontx(fx16G, 0, buffer_fx16G, &fontWidth_fx16G, &fontHeight_fx16G);

	// set up screen
	display_t display;
	display_init(&display);
	display_set_flip(&display, true, true);
	initPrintData(&display, fx16G);

	// initialise variabes
	uint8_t
		Amplitude = 0,
		Frequency = 0,
		Command;
	Command = ( (Amplitude << 4) + Frequency );

	// set up IIC0 on the arduino SCL and SDA lines
	switchbox_set_pin(IO_AR1, SWB_IIC0_SCL);
	switchbox_set_pin(IO_AR0, SWB_IIC0_SDA);
	iic_init(IIC1);
	iic_reset(IIC1);

	//		START OF CODE THAT ACTUALLY DOES STUFF
	for(;;){

		// get command
		iic_read_register(IIC1, ALGORITHM_ADDRESS, 0, &Command, 1 );
		Amplitude = Command >> 4;
		Frequency = Command & 0x0f;

		printData(&display, fx16G, Command);

		sleep_msec(10);
	}

	//		DESTROY EVERYTHING
	display_destroy(&display);
	pynq_destroy();
	return EXIT_SUCCESS;
}
