#include <libpynq.h>
#include <stdio.h>

#define CRYING_ADDRESS    0x10
#define HEARTBEAT_ADDRESS 0x20
#define ALGORITHM_ADDRESS 0x30
#define ACTUATOR_ADDRESS  0x40

#define BACKGROUND_COLOR RGB_BLACK
#define TEXT_COLOR RGB_GREEN

#define FONTSIZE 16
#define FONTWIDTH 8

#define LCD_TEXT_X 48
#define LCD_TEXT_Y 114

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
	displayDrawFillRect(
		display,
		x,
		y,
		x + size * FONTWIDTH,
		y + FONTSIZE - 1,
		BACKGROUND_COLOR
	);
	displayDrawString(
		display,
		fx16G,
		x,
		y + FONTSIZE,
		(uint8_t *)str,
		TEXT_COLOR
	);
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
		(uint8_t *)"Heartbeat:   bpm",
		TEXT_COLOR
	);
}

void printData(
	display_t *display,
	FontxFile *fx16G,
	uint8_t  heartbeat
){
	printInt(
		display,
		fx16G,
		LCD_TEXT_X + 10 * FONTWIDTH,
		LCD_TEXT_Y,
		"%03d",
		heartbeat
	);
}

void tempHeartbeatUpdateFunc(uint8_t *heartbeat){
	if(get_button_state(0))
		*heartbeat -= 1;
	if(get_button_state(1))
		*heartbeat += 1;
	if(get_button_state(2))
		*heartbeat -= 50;
	if(get_button_state(3))
		*heartbeat += 50;
}

int main(){
	// It ain't a bo'o o' wo'er init?
	// Ra'er sunny wea'er init?
	pynq_init();
	buttons_init();

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
		Heartbeat = 40;

	// set up IIC0 on the arduino SCL and SDA lines
	switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
	switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
	iic_init(IIC0);
	iic_reset(IIC0);
	iic_set_slave_mode (
		IIC0,
		HEARTBEAT_ADDRESS,
		(uint32_t *)&Heartbeat,
		1
	);

	//		START OF CODE THAT ACTUALLY DOES STUFF
	for(;;){

		// set heartbeat
		tempHeartbeatUpdateFunc(&Heartbeat);

		iic_slave_mode_handler(IIC0);

		printData(&display, fx16G, Heartbeat);

		sleep_msec(10);
	}

	//		DESTROY EVERYTHING
	display_destroy(&display);
	buttons_destroy();
	pynq_destroy();
	return EXIT_SUCCESS;
}
