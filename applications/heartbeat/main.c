#include <libpynq.h>
#include <stdio.h>

#define CRYING_ADDRESS    0x10
#define HEARTBEAT_ADDRESS 0x20
#define ACTUATOR_ADDRESS  0x40

#define BACKGROUND_COLOR RGB_BLACK
#define TEXT_COLOR RGB_GREEN

void printData(
	display_t *display,
	FontxFile *fx16G,
	uint8_t  heartbeat
){
	char str[16]="";
	sprintf(str, "Heartbeat %d", heartbeat);
	displayFillScreen(display, BACKGROUND_COLOR);
	displayDrawString(display, fx16G, 120, 16, (uint8_t *)str, TEXT_COLOR);
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

	// set up screen
	display_t display;
	display_init(&display);
	display_set_flip(&display, true, true);
	displayFillScreen(&display, BACKGROUND_COLOR);

	// set up screen font
	uint8_t
		buffer_fx16G[FontxGlyphBufSize],
		fontWidth_fx16G,fontHeight_fx16G;
	FontxFile fx16G[2];
	InitFontx(fx16G, "/boot/ILGH16XB.FNT", "");
	GetFontx(fx16G, 0, buffer_fx16G, &fontWidth_fx16G, &fontHeight_fx16G);

	// initialise variabes
	uint8_t
		Heartbeat = 0;

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
