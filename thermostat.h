// Display related Constants
//Font Setup
#define	DISP_HT_GLCD_FONT			 8	//Short -2
#define	DISP_HT_TAHOMA_FONT		16	//Short
#define	DISP_HT_CAL_36_FONT		33	//Short -6
#define	DISP_HT_CAL_72_FONT		82	//short	-12
#define	DISP_HT_MSSS_14_FONT	20	//Short -16
#define	DISP_HT_MSSS_26_FONT	35	//Short	-8?
#define DISP_HT_MSSS_12_FONT  17

//Time Setup
#define TIME_DISPLAY_X		16
#define TIME_DISPLAY_Y		1
#define TIME_DISPLAY_WIDTH 210
#define TIME_F_SCALE			1
#define TIME_ZONE_OFFSET	-5
#define	TIME_DST_OFFSET		1

#define	TIME_FONT					MSFTSS_14
#define TIME_COLOR				ILI9341_YELLOW

// Heat indicator setup
#define HEAT_DISPLAY_X		227
#define HEAT_DISPLAY_Y		0
#define HEAT_ON_COLOR			ILI9341_RED
#define HEAT_OFF_COLOR		ILI9341_GREY
#define	HEAT_FONT					MSFTSS_14
#define	HEAT_F_SCALE			1

// Occupants Setup
#define MAX_OCCUPANTS	4  //How many IP addresses to track to see if they are home
#define OCC_INDEX			4
#define OCC_INITIAL		5

#define OCCUPANTS_X			1  //212
#define OCCUPANTS_Y			0
#define	OCCUPANT_FONT			GLCDFONT  //7 pix high
#define	OCCUPANT_F_SCALE	1
#define OCCUPANT_COLOR	ILI9341_CYAN

#define LANCE 		'L' //L
#define MICHELLE 	'M'	//M
#define ANDREW		'A'	//A
#define ROBERT		'R'	//R
#define SPACE			' '	//Space

//Programming Setup
#define PROG_DISP_Y				24
#define	PROG_DISPP_X			32
#define	PROG_DISP_TEMP_X	108
#define	PROG_DISP_TIME_X	150

//Region Setup
//Common
#define	LARGE_TEMP_FONT		CALIBRI_72
#define LARGE_TEMP_F_SCALE	1
#define LARGE_TEMP_AVG_WIDTH 24      // Average width of unscaled digit in font.  Use for centering

#define DISP_BRIGHT_HIGH	255  //PWM values
#define DISP_BRIGHT_MED	155    // Nicely bright but not overpowering
#define DISP_BRIGHT_DIM	20
#define DISP_BRIGHT_OFF	0

#define L_TDISP_WIDTH     152
#define L_TDISP_HEIGHT    DISP_HT_CAL_72_FONT - 12
#define L_XINDENT_RECT_X	64
#define BG_COLOR				ILI9341_BLACK

//Setpoint
#define SETPOINT_COLOR	FORESTGREEN
#define L_SETPOINT_RECT_Y	42

//Indoor

#define	INDOOR_COLOR		ILI9341_RED
#define L_INDOOR_RECT_X			64
#define L_INDOOR_RECT_Y			130
#define IN_HUMIDITY_RECT_X		170
#define IN_HUMIDITY_RECT_Y	 	170
#define HUMIDITY_FONT				MSFTSS_26
#define HUMIDITY_FONT_SCALE	1
#define HUMDISP_WIDTH   	66
#define HUMDISP_HEIGHT		DISP_HT_MSSS_26_FONT - 7


//Outdoor
#define	OUTDOOR_COLOR		0x6BFA                //SLATEBLUE
#define O_OUTDOOR_INDENT_RECT_X 30
#define O_OUTDOOR_RECT_Y	204

#define O_OUTDOOR_CTR_X   			83
#define O_OUTDOOR_DISP_WIDTH   114
#define O_OUTDOOR_DISP_HEIGHT   DISP_HT_CAL_36_FONT

//#define OUT_HUMIDITY_RECT_X		176
//#define OUT_HUMIDITY_RECT_Y	 	302

// Weather
#define	WEATHER_TEMP_FONT			CALIBRI_36
#define WEATHER_TEMP_F_SCALE		1
#define WEATHER_TEMP_AVG_WIDTH 14

#define FHI_COLOR					0xFA28
#define FLO_COLOR					DARKCYAN
#define L_ALERT_RECT_Y		207
#define L_HILO_RECT_Y			217
#define L_OTEMP_RECT_Y		245
#define L_REALFEEL_RECT_Y	285

#define REALFEEL_RECT_X 	30
#define FEELS_COLOR_H			ORANGE
#define FEELS_COLOR				CADETBLUE
#define FEELS_FONT				MSFTSS_12       //TAHOMA_12
#define PRECIP_COLOR			ROYALBLUE
#define PRECIP_FONT				MSFTSS_14
#define HILO_FONT			  	MSFTSS_14
#define O_HUMIDITY_COLOR	ORANGE
#define O_HUMIDITY_FONT		MSFTSS_12
#define O_HUMIDITY_RECT_X	115
#define O_HUMIDITY_RECT_Y	263
#define O_HUMIDITY_WIDTH	30
#define O_HUMDISP_HEIGHT  10
#define L_PRECIP_RECT_Y		302
#define L_PRECIP_RECT_X		146
#define PRECIP_RECT_WIDTH	94
#define WEATHER_ICON_SET  "WLG"
#define W_ICON_RECT_SIZE  96
#define WCONDITIONS_X     144
#define WCONDITIONS_Y     205

#define WIND_COLOR				CADETBLUE
#define WIND_COLOR_HL			ORANGE
#define WIND_FONT					MSFTSS_12

//#define L_WIND_RECT_Y			295
#define T_WIND_RECT_X			24
#define WIND_RECT_X				24
#define WIND_RECT_Y	 			305
#define WIND_WIDTH	 			123
#define WIND_HEIGHT  			DISP_HT_MSSS_12_FONT

// Font Settings

#define	SIDE_TITLE_FONT		MSFTSS_14
#define	SIDE_TITLE_F_SCALE	1


#define HEXDEGREES 0xB0
// Constants for Color Scheme
#define ILI9341_DKGREEN 0x03c2
#define ILI9341_GREY    0xD75D
#define ILI9341_PURPLE  0xD976


#define HEAT_TYPE_FHW TRUE  //Use this is you want to add to the temperature to
// prevent the heat from running too long what the
//heat source will continue to heat after the furnace
//is off. Use with Forced Hot Water, Steam,Convection
#define HEAT_ADVANCE_VAL 1  // the default amount to add to the temperature reading
// if using the FHW advance

//uint8_t lkpOccTest = 0;
#define TEMP_SENSOR	 0x27

//Pin Definitions
//#define FAN_PIN     A0
#define HEAT_PIN    		A1
#define POT_PIN    		 	A0
#define PIR_PIN    			A7
#define REPAINT_SWITCH	D7
#define SDA_PIN     		D0
#define SCL_PIN     		D1
#define DISP_BACKLIGHT	RX
#define SD_CS						D6

#define SP_LOW 					40
#define SP_HIGH 				90
#define SP_DEFAULT		 	68
#define LOG_INTERVAL		15U*60U  //10 minutes   go up to 15 later
#define LOG_ADJUST      0U*60U   //Push forward 5 minutes to catch recent weather update
//EEPROM
#define DESIRED_TEMP_EEPROM_ADDR 	0x01
#define HOLD_TEMP_EEPROM_ADDR 		0x02
#define DST_STATUS_ADDR						0x03




#define MAX_WEATHER_TRIES			 10  // every 5 is one second



typedef enum {
    LEARN,
    PROG,
    TEMPHOLD,
    HOLD,
    HOLIDAY,
    AWAY
} prog_state;

//Prototypes

void displaySidewaysTitles();
void displayHeatIndicator();
void displayOccupants();
void displayTime();
void displaySetPoint();
void displayProgInfo();
void saveSetPoint();
void loadSetPoint();
int setSetPoint(int );
int setSetPointFromString(String);
void displayTemperature();
void displayOutdoorTemp();
void displayStrings(char *,char *, int , int , int , int , int16_t  , int16_t , int16_t , int16_t , int16_t , uint16_t , bool  );
void displayStrings(char *, int , int , int , int16_t  , int16_t , int16_t , int16_t , int16_t , uint16_t );
int setOutdoorTemperature(int );
int setConditsFromString(String );
int setForecastFromString(String );
int settInfoReadFromString(String );
int setInfoTemp(int);
void paintScreen();
void bmpDraw(char *, uint8_t , uint16_t );
void gotWeatherReply(const char *, const char *);
void gotAppendReply(const char *, const char *) ;

const uint8_t ADC_DIM = 16;
struct data_t {
  unsigned long time;
  unsigned short adc[ADC_DIM];
  float efrr;
};
// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4)/sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM*sizeof(data_t);

struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};
void logData();
void updateTempInfo();
void dumpData();
void checkOverrun();
void binaryToCsv();
void error(const char* );
void fatalBlink();
void acquireData(data_t* );
//void acquireData(data_t* );
void printData(Print* , data_t*);
void printHeader(Print*);
long runningAverage(int);