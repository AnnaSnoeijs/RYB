#include <libpynq.h>
#include <stdio.h>

#define CRYING_ADDRESS    0x10
#define HEARTBEAT_ADDRESS 0x20
#define ACTUATOR_ADDRESS  0x40

#define AMPLITUDE_MAX 5
#define FREQUENCY_MAX 5

#define BACKGROUND_COLOR RGB_BLACK
#define TEXT_COLOR RGB_GREEN

struct stress_t {
	uint8_t heartbeat;
	uint8_t volume;
	uint8_t stress;
};


void calcStress( struct stress_t *ToDo ){
	ToDo->stress = ToDo->heartbeat / 2 - 20;
	if( ToDo->stress > 50 )return;
	//TO DO: ADD CRY TO CALCULATION IF STRESS BELOW 50%
}


void printData(
	display_t *display,
	FontxFile *fx16G,
	struct stress_t Matrix[AMPLITUDE_MAX][FREQUENCY_MAX],
	uint8_t  command
){
	char
		str[512]="",
		numstr[8];
	uint8_t
		a,
		f;

	// clear screen
	displayFillScreen(display, BACKGROUND_COLOR);

	// display matrix
	for( a = 0; a < AMPLITUDE_MAX; a++){
		for( f = 0; f < FREQUENCY_MAX; f++){
			sprintf(numstr, "%d ", Matrix[a][f].stress);
			strcat(str, numstr);
		}
		displayDrawString(display, fx16G, 0, 16*(a+1), (uint8_t *)str, TEXT_COLOR);
		str[0]='\0';
	}

	// display rest of info
	a = command >> 4;
	f = command & 0x0f;
	sprintf(str, "Heartbeat %d", Matrix[a][f].heartbeat);
	displayDrawString(display, fx16G, 120, 16, (uint8_t *)str, TEXT_COLOR);
	sprintf(str, "Volume %d", Matrix[a][f].volume);
	displayDrawString(display, fx16G, 120, 32, (uint8_t *)str, TEXT_COLOR);
	sprintf(str, "Stress %d", Matrix[a][f].stress);
	displayDrawString(display, fx16G, 120, 48, (uint8_t *)str, TEXT_COLOR);
	sprintf(str, "Amplitude %d", a);
	displayDrawString(display, fx16G, 120, 64, (uint8_t *)str, TEXT_COLOR);
	sprintf(str, "Frequency %d", f);
	displayDrawString(display, fx16G, 120, 80, (uint8_t *)str, TEXT_COLOR);
}

void tempCommandUpdateFunc(uint8_t *command){
	if(get_button_state(0))
		*command -= 1;
	if(get_button_state(1))
		*command += 1;
	if(get_button_state(2))
		*command -= 0x10;
	if(get_button_state(3))
		*command += 0x10;
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

	// set up IIC0 on the arduino SCL and SDA lines
	switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
	switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
	iic_init(IIC0);

	// initialise variabes
	uint8_t
		Amplitude = 3,
		Frequency = 5,
		Heartbeat = 0,
		Volume = 0,
		Command;
	Command = ( (Amplitude << 4) + Frequency );

	struct stress_t Matrix[AMPLITUDE_MAX][FREQUENCY_MAX];

	//		START OF CODE THAT ACTUALLY DOES STUFF
	for(;;){ // stuff is gonna loop but this is just temporary

		// set command
		tempCommandUpdateFunc(&Command);
		Amplitude = Command >> 4;
		Frequency = Command & 0x0f;

		// send command to actuator submodule
		iic_write_register(IIC0, ACTUATOR_ADDRESS, 0, &Command, 1 );

		// read from crying and heartbeat submodules
		iic_read_register(IIC0, CRYING_ADDRESS, 0, &Volume, 1 );
		Matrix[Amplitude][Frequency].volume = Volume;
		iic_read_register(IIC0, HEARTBEAT_ADDRESS, 0, &Heartbeat, 1 );
		Matrix[Amplitude][Frequency].heartbeat = Heartbeat;

		printData(&display, fx16G, Matrix, Command);

		sleep_msec(1000);
	}

	//		DESTROY EVERYTHING
	display_destroy(&display);
	buttons_destroy();
	pynq_destroy();
	return EXIT_SUCCESS;
}
