#include <libpynq.h>
#include <stdio.h>

#define CRYING_ADDRESS    0x10
#define HEARTBEAT_ADDRESS 0x20
#define ALGORITHM_ADDRESS 0x30
#define ACTUATOR_ADDRESS  0x40

#define AMPLITUDE_MAX 5
#define FREQUENCY_MAX 5

#define BACKGROUND_COLOR RGB_BLACK
#define TEXT_COLOR       RGB_PURPLE

#define FONTSIZE 16
#define FONTWIDTH 8

#define LCD_MATRIX_X 40
#define LCD_MATRIX_Y 40
#define LCD_TEXT_X 48
#define LCD_TEXT_Y 130
#define LCD_TEXT_NUMBERS_X LCD_TEXT_X + 10 * FONTWIDTH

struct stress_t {
	uint8_t heartbeat;
	uint8_t volume;
	uint8_t stress;
};


void calcStress( struct stress_t *Stress ){
	if( Stress->heartbeat < 40 ){
		Stress->stress = 0;
		return;
		}
	Stress->stress = (Stress->heartbeat - 40) / 2;
	if( Stress->stress > 50 )return;
	//TO DO: ADD CRY TO CALCULATION IF STRESS BELOW 50%
}

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
	char str[512]="";
	uint8_t a, f;

	// clear screen
	displayFillScreen(display, BACKGROUND_COLOR);
	//displayFillScreen(display, RGB_BLUE);

	// display matrix
	for( a = 0; a < AMPLITUDE_MAX; a++){
		for( f = 0; f < FREQUENCY_MAX; f++){
			strcat(str, "idk ");
		}
		displayDrawString(display, fx16G, LCD_MATRIX_X, LCD_MATRIX_Y + FONTSIZE*(a + 1), (uint8_t *)str, TEXT_COLOR);
		str[0]='\0';
	}

	// display rest of info
	displayDrawString(display, fx16G, LCD_TEXT_X, LCD_TEXT_Y + 1 * FONTSIZE, (uint8_t *)"Heartbeat:", TEXT_COLOR);
	displayDrawString(display, fx16G, LCD_TEXT_X, LCD_TEXT_Y + 2 * FONTSIZE, (uint8_t *)"   Volume:", TEXT_COLOR);
	displayDrawString(display, fx16G, LCD_TEXT_X, LCD_TEXT_Y + 3 * FONTSIZE, (uint8_t *)"   Stress:", TEXT_COLOR);
	displayDrawString(display, fx16G, LCD_TEXT_X, LCD_TEXT_Y + 4 * FONTSIZE, (uint8_t *)"Amplitude:", TEXT_COLOR);
	displayDrawString(display, fx16G, LCD_TEXT_X, LCD_TEXT_Y + 5 * FONTSIZE, (uint8_t *)"Frequency:", TEXT_COLOR);
}

void printData(
	display_t *display,
	FontxFile *fx16G,
	struct stress_t Matrix[AMPLITUDE_MAX][FREQUENCY_MAX],
	uint8_t  command
){
	uint8_t a, f;

	// display current point in matrix
	a = command >> 4;
	f = command & 0x0f;
	printInt(
		display,
		fx16G,
		LCD_MATRIX_X + 4 * FONTWIDTH * f,
		LCD_MATRIX_Y +     FONTSIZE  * a,
		"%02d%%",
		Matrix[a][f].stress
	);

	// display rest of info
	printInt(display, fx16G, LCD_TEXT_NUMBERS_X, LCD_TEXT_Y + 0 * FONTSIZE, "%dbpm  ", Matrix[a][f].heartbeat);
	printInt(display, fx16G, LCD_TEXT_NUMBERS_X, LCD_TEXT_Y + 1 * FONTSIZE, "%ddB  ", Matrix[a][f].volume);
	printInt(display, fx16G, LCD_TEXT_NUMBERS_X, LCD_TEXT_Y + 2 * FONTSIZE, "%d%%  ", Matrix[a][f].stress);
	printInt(display, fx16G, LCD_TEXT_NUMBERS_X, LCD_TEXT_Y + 3 * FONTSIZE, "%d", a+1);
	printInt(display, fx16G, LCD_TEXT_NUMBERS_X, LCD_TEXT_Y + 4 * FONTSIZE, "%d", f+1);
}

void tempCommandUpdateFunc(uint8_t *command){
	if(get_button_state(0) && (*command & 0x0f))
		*command -= 1;
	if(get_button_state(1) && (*command & 0x0f) < 4 )
		*command += 1;
	if(get_button_state(2) && (*command & 0xf0))
		*command -= 0x10;
	if(get_button_state(3) && (*command < 0x40))
		*command += 0x10;
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
		Amplitude = 2,
		Frequency = 3,
		Heartbeat = 0,
		Volume = 0,
		Command;
	Command = ( (Amplitude << 4) + Frequency );

	// set up IIC0 on the arduino SCL and SDA lines
	switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
	switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
	iic_init(IIC0);
	iic_reset(IIC0);

	// and the stupid iic1 slave because we can't write to slaves
	switchbox_set_pin(IO_AR1, SWB_IIC1_SCL);
	switchbox_set_pin(IO_AR0, SWB_IIC1_SDA);
	iic_init(IIC1);
	iic_reset(IIC1);
	iic_set_slave_mode (
		IIC1,
		ALGORITHM_ADDRESS,
		(uint32_t *)&Command,
		1
	);

	struct stress_t Matrix[AMPLITUDE_MAX][FREQUENCY_MAX];

	//		START OF CODE THAT ACTUALLY DOES STUFF
	printf("actually started doing stuff\n");
	for(;;){
		// read from crying and heartbeat submodules
		iic_read_register(IIC0, CRYING_ADDRESS, 0, &Volume, 1 );
		Matrix[Amplitude][Frequency].volume = Volume;
		iic_read_register(IIC0, HEARTBEAT_ADDRESS, 0, &Heartbeat, 1 );
		Matrix[Amplitude][Frequency].heartbeat = Heartbeat;

		// calculate stress
		calcStress( &Matrix[Amplitude][Frequency] );

		// set command
		tempCommandUpdateFunc(&Command);
		Amplitude = Command >> 4;
		Frequency = Command & 0x0f;

		// send command to actuator submodule
		Command += 0x11;
		//iic_write_register(IIC0, ACTUATOR_ADDRESS, 1, &Command, 1); // Should work but doesn't
		/* stupid temp actuator fix*/
		iic_slave_mode_handler(IIC1);
		/* end of temp actuator fix*/
		Command -= 0x11;

		printData(&display, fx16G, Matrix, Command);

		sleep_msec(10);
	}

	//		DESTROY EVERYTHING
	printf("destroying everything c:\n");
	display_destroy(&display);
	buttons_destroy();
	pynq_destroy();
	return EXIT_SUCCESS;
}
