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

void printData(
	display_t *display,
	FontxFile *fx16G,
	uint8_t  command
){
	char
		str[512]="";
	uint8_t
		a,
		f;

	// clear screen
	displayFillScreen(display, BACKGROUND_COLOR);

	// display rest of info
	a = command >> 4;
	f = command & 0x0f;
	sprintf(str, "Amplitude %d", a);
	displayDrawString(display, fx16G, 120, 16, (uint8_t *)str, TEXT_COLOR);
	sprintf(str, "Frequency %d", f);
	displayDrawString(display, fx16G, 120, 32, (uint8_t *)str, TEXT_COLOR);
}

int main(){
	// It ain't a bo'o o' wo'er init?
	// Ra'er sunny wea'er init?
	pynq_init();

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
		Amplitude = 3,
		Frequency = 5,
		Command;
	Command = ( (Amplitude << 4) + Frequency );

	// set up IIC0 on the arduino SCL and SDA lines
	switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
	switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
	iic_init(IIC0);
	iic_reset(IIC0);

// don't work :c /*
	iic_set_slave_mode (
		IIC0,
		ACTUATOR_ADDRESS,
		(uint32_t *)&Command,
		1
	);
/**/
	//		START OF CODE THAT ACTUALLY DOES STUFF
	for(;;){

		// get command
		/* is brokeys :c
		iic_slave_mode_handler(IIC0);
		/*/
		iic_read_register(IIC0, ALGORITHM_ADDRESS, 0, &Command, 1 );
		/**/
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
