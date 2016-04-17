/***************************************************************
This is a variation on the thermostat code written by Particle.  The main difference is that it uses an Adafruit
2.2" TFT Display with an integrated flash memory micro sd card. It also uses the multifont Adafruit graphics library.
I created special version of 36 point Calibri font that only supports digits and a few symbols necessary for
displaying the temperature.
// Constructor when using hardware SPI.  Faster, but must use specific SPI pins (http://docs.particle.io/#/hardware):
// A0 : POT for Setpoint adjustment
// A1 : Relay for Heat activiation
// A2 : SS(Slave Select) for DISPLAY CS - Chip select (aka slave select)
// A3 : SCK(Serial Clock)
// A4 : MISO(Master In Slave Out)
// A5 : MOSI(Master Out Slave In)
// DAC :
// TX : BL Backlight (allows dimming) with PWM
// The other pins are:  These are the ones passed as arguments to Adafruit_ILI9340
// D0 : (SDA) Thermometer/Hygrometer
// D1 : (SCL)  Thermometer/Hygrometer
// D2 : Open
// D3 : Open
// D4 : dc - D/C or A0 on the screen (Command/Data switch)
// D5 :  rst - Reset pin for display
// D6 : SD_CS
// D7 :	Switch for UI /Repaint

*******************************************************************
*/
#pragma SPARK_NO_PREPROCESSOR   // This will direct Arduino preprocessor to not create prototypes for functions.
																// setting this means you must manually make function prototypes according to C++ rules
																// This can avoid some incompatibilities with some libraries

//#define USE_SERIAL_OUTPUT  1  // Use to eliminate Serial display to save bytes
//#define LH_DEBUG 			1
//#define I2C_DEBUG 		1
#define POT_DEBUG 			1
#define PRINT_VERSION 	1
//#define __STDC_WANT_LIB_EXT1__ 1

#include "application.h"
#include "Adafruit_mfGFX.h"
#include "Adafruit_ILI9341.h"
#include "fonts.h"
#include "565_color_table.h"
#include "thermostat.h"
//#include <string.h>
//#include <stdio.h>
#include <math.h>
#include "SdFat.h"

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

// Pick an SPI configuration.
// See SPI configuration section below (comments are for photon).
#define SD_CS D6
#define SPI_CONFIGURATION 0
//------------------------------------------------------------------------------
// Setup SPI configuration.
#if SPI_CONFIGURATION == 0
// Primary SPI with DMA
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFat sd;
const uint8_t chipSelect = SD_CS;
#endif  // SPI_CONFIGURATION


uint8_t Occupants[MAX_OCCUPANTS][6] {
	{192,168,180,200,0,ROBERT},
	{192,168,180,220,0,LANCE},
	{192,168,180,215,0,MICHELLE},
	{192,168,180,211,0,ANDREW},
}; // For the second Array, indexes 0-3 are the IP Address, [4] is an int that is 0 if not home or >0 if they are home.


//Timing and Update frequency

unsigned int previousSecs = 0UL;   //Used for Time update to prevent overdoing display
unsigned int prevOccSecs =  0UL;

unsigned int displayUpdInterval = 60U;  // 1 Minute
unsigned int updateOccInterval	= 15U*61U;  // 15  61  seconds to avoid clashing with logging on the tens
unsigned int currentSecs = Time.now();
unsigned int nextLogTime = currentSecs;
unsigned int nextWeatherTime = nextLogTime;
const float dailySeconds = 86400.0;  // Seconds per day  (A1/86400)+(25569)
const float dayConv = 25569.0;       // For Sheet date conversion days between 1/1/1900 and 1/1/1970 

int indoorTemperature = 55;
int desiredSetPoint = 55;
int outdoorTemperature = 55;
float percentHumidity = 10.0;
int	outRelH = 5;
int	outDewp = 5;
int	outFeelf = 55;
int pop = 0; //percent chance of precipitation
int estFuelLevel = 0;
float fuelRunRate = 0.325; //gallons per sec
//bool isHeatOn = false;
bool isHoldOn = false;
int holdTemp = 50;
String	outIcon = "chancestorms";
String	outCondit = "chancestorms";
//char swalert[36] = "Weather Alert Place here";
String swalert =  "Weather Alert Place";
char wIcon[36] = "chancestorms";
int tInfoRead = 0;
int diffMax = 0;
int heatSecs = 0;
unsigned int cumHeatOn = 0U; //Holds cumulative millis that heat has been on since last log.
unsigned int prevCumHeatOn =0U;
static double sheetStamp = 0;
unsigned int weatherObsTime = 0;
int numHome = 0;
int prevHeat = 0;
int isHeatOn = 0;  // convert this to a char array to get several values in one (fan on heat on cool on)
int flowTemp = -80;
int fhighTemp = 100;
byte icstatus = 0;
bool isFanOn = false;
bool motionDetected = false;
int pot = 0;
int isdst = 1;
int hourForec1;
int hourForec3;
int windSpeed;
int windGust;
char windDir[4] = "NW";

//  hf1,hf3,ws,wd,wg

int lastChangedPot = -80;
int potChgCount = 0;
char resultstr[622];
char results2[622];
char dataString[200];
char tempStr[64] =  "123|567";
char tempStr1[64] = "123|567";
//char condits[64];
//
int resets = 0;
//char pheat[6];
char pheat[8] = "0.1234";
String timStr = "Fri Sep 32 ,10:16 PM";
//char meridian[4] = " AM";
//strcpy(pheat, "0.1234");
int potmax , potmin ;
int potDiff = 0;
int dstOffset = 0;
prog_state pState = PROG;
uint16_t read16(File &f);
uint32_t read32(File &f);
//data_t data;

// Is the following block an artifact of the fast data logger
//==============================================================================
// Start of configuration constants.
//==============================================================================
//Interval between data records in microseconds.
const uint32_t LOG_INTERVAL_USEC = 900000;
//------------------------------------------------------------------------------
// Pin definitions.
//
// SD chip select pin.
const uint8_t SD_CS_PIN = D6;
//
// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for SD write
// overrun errors and logging continues.
const int8_t ERROR_LED_PIN = -1;
//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in blocks.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
const uint32_t FILE_BLOCK_COUNT = 256000;

// log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "tlog"
//------------------------------------------------------------------------------
// Buffer definitions.
//
// The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT additional
// buffers.
//
#ifndef RAMEND
// Assume ARM. Use total of nine 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 8;
//
#elif RAMEND < 0X8FF
#error Too little SRAM
//
#elif RAMEND < 0X10FF
// Use total of two 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 1;
//
#elif RAMEND < 0X20FF
// Use total of five 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 4;
//
#else  // RAMEND
// Use total of 13 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 12;
#endif  // RAMEND
//==============================================================================
// End of configuration constants.
//==============================================================================
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_log.bin"

// Size of file base name.  Must not be larger than six.
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

//SdFat sd;

SdBaseFile binFile;


const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 2;

block_t* emptyQueue[QUEUE_DIM];
uint8_t emptyHead;
uint8_t emptyTail;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead;
uint8_t fullTail;

// Advance queue index.
inline uint8_t queueNext(uint8_t ht) {
  return ht < (QUEUE_DIM - 1) ? ht + 1 : 0;
}
//==============================================================================

//
Adafruit_ILI9341 tft = Adafruit_ILI9341(A2, D4, D5 );



/**************************************************************************************************************************

 						 SETUP  Always the first call for Particle Devices.

**************************************************************************************************************************/

//SYSTEM_MODE(MANUAL); // Great for debugging when wifi may be causing voltage drops
void setup()
{
	Wire.begin();
	tft.begin();
	Serial.begin(9600);
	#ifdef SERIAL_DEBUG
	  Serial.println("Type any character to start");
 	  while (Serial.read() <= 0) {}
 		delay(400);  // catch Due reset problem
  #endif
  	//while (!Serial.available()) Particle.process();
	//Particle.process() replaces SPARK_WLAN_Loop()
  //while (Serial.read() >= 0) {}	// Maybe take the input to know serial is on
 	pinMode(HEAT_PIN, OUTPUT);
	pinMode(POT_PIN, INPUT);
	pinMode(PIR_PIN, INPUT);
	pinMode(REPAINT_SWITCH, INPUT_PULLUP);  //Pullup activates the internal resistor to pul the voltage weakly up to make the switch more responsive.
	pinMode(SDA_PIN, INPUT_PULLUP);  //Use real resistors instead
	pinMode(SCL_PIN, INPUT_PULLUP);
	//digitalWrite(REPAINT_SWITCH,HIGH);

 	potmax = potmin = 4096 - analogRead(POT_PIN);
 	pot = (int) runningAverage(potmax);
 	
	tft.fillScreen(BG_COLOR);
	tft.setCursor(0, 0);
//
	int dstStat = EEPROM.read(DST_STATUS_ADDR);
	if (dstStat > 1 || dstStat < 0)
		{ 
			isdst = 1;
			EEPROM.update(DST_STATUS_ADDR, (uint8_t) isdst);  // Update only writes if value is different
		}
	else
		{
			isdst = dstStat;
	 	};
//
	Time.zone(TIME_ZONE_OFFSET + isdst);                                // Add intelligence here to switch between Daylight and Standard Time (Weather Underground may help)
  // Initialize the SD.
	
	Serial.print("Initializing SD card...");
  if (!sd.begin(SD_CS)) 
  	{
   	  Serial.println("failed!");
   	 // errorHalt("SD begin failed");
 	 }
  Serial.println("OK!");
  	
	pinMode(DISP_BACKLIGHT, OUTPUT);
	analogWrite(DISP_BACKLIGHT,DISP_BRIGHT_DIM);  //Set brightness of screen first  USE PWM
//	int Occupant1 = 0;
// Limit names to 12
	Particle.function("set_setpoint", setSetPointFromString);			// Simple function which can set the setpoint externally.
//	Particle.function("set_condits" , setConditsFromString);			// Deprecated renamed to gotWeatherReply and converted to webhook This opns up more space for returned data // (1) Sets all weather info conditions and forecast
//	Particle.function("set_forecast", setForecastFromString);			//Deprecated gotWeatherReply webhook now handles conditins and forecast//Deprecated, replaced by NComboWeather in google script.
//	Particle.function("set_ti_read",  settInfoReadFromString);    // Deprecated now triggered from inside with logging to flash drive as well.  Much more accurate and reliable // (3)could be eliminated if trigger from inside


 	Particle.variable("setpoint_tmp", &desiredSetPoint, INT);
 	Particle.variable("indoor_temp", &indoorTemperature, INT); 	// Can be reported in JSON with other info as well. DO it with internal info read or created by Photon
		// sprintf(resultstr, "{\"s\":%d,\"i\":%d,\"o\":%d,\"l\":%d,\"h\":%d,\"co\":%d,\"or\":%d}", desiredSetPoint, indoorTemperature, outdoorTemperature,flowTemp,fhighTemp,heatSecs,outRelH );
		// indoorTemperature,indoorHumidity,isHeatOn,isFanOn,setpointByProgram, occupiedFence,occupiedMotion
	Particle.variable("outdoor_temp", &outdoorTemperature, INT);
	Particle.variable("is_heat_on", &isHeatOn, INT);  				// Could change to string with heat on and how many seconds today
	Particle.variable("tempInfo", resultstr, STRING); 					// (2) When reading this varible, then write a TRUE to tInfoRead to mark that it has been read
		//\"spt\":%d,\"int\":%d,\"inh\":%d,\"hos\":%d,\"efl\":%d,\"efl\":%d,\"frr\":%d,\"fon\":%d,\"hon\":%d,\"ocf\":%d,\"ocm\":%d,\"hon\":%d,\"hot\":%d,\"ovo\":%d,\"ovt\":%d,\"ffq\":%d,\"ffd\":%d,\"fuu\":%d,\"evt\":%d,\"fmc
 	Particle.variable("infoLog", results2, STRING);				//(deprecated?)The variable does not need an & if it is defined as an array of char since this is the native

//WEBHOOKS
	Particle.subscribe("hook-response/gdweather_lkp", gotWeatherReply, MY_DEVICES);  //replaces set_condits and set_forecast
	// Lets listen for the hook response
	delay(200);
  Particle.subscribe("hook-response/add_2sheet_lkp", gotAppendReply, MY_DEVICES);  // Works with infoLog to post a new row of logged data to sheet


	loadSetPoint();
	//pinMode(FAN_PIN, OUTPUT);

	digitalWrite(REPAINT_SWITCH,HIGH);

  // publish the event that will trigger our Webhook
  Particle.publish("gdweather_lkp");
  delay(200); // Don't worry, the weather will display when triggered

	paintScreen();
	#ifdef PRINT_VERSION   //Print version in Alert area
		tft.setRotation(3);
		sprintf(tempStr1,"%s, Fmem: %lu",System.version().c_str(), System.freeMemory() );
 		displayStrings(tempStr1, 1, OCCUPANT_FONT, RED, 20 , 17, 20, 120, 12, BG_COLOR );
  	//delay(1000);
		tft.setRotation(0);	tft.setTextColor(OCCUPANT_COLOR);  //Return to normal
	#endif
	//	nextWeatherTime = (currentSecs - (currentSecs % (LOG_INTERVAL)) + (LOG_INTERVAL) -120);  
	
	nextLogTime = currentSecs - (currentSecs % (LOG_INTERVAL)) + (LOG_INTERVAL) +(LOG_ADJUST);// 15 Minutes;((1458569013)+900)-MOD(A66,900)
	nextWeatherTime = nextLogTime - 30;  // Update the weather 2 minutes before the log

}

/**************************************************************************************************************************

 											 LOOP  The equivalent of main() for Particle .ino programs

**************************************************************************************************************************/

void loop()
{
	static int wait = 0;
	//static int potreads = 0;
	//unsigned long pottime = millis();
	//tft.fillScreen(BG_COLOR);

	if (!wait)
	{
		wait = 1000;
		#ifdef PRINT_VERSION   //Print version in Alert area
			tft.setRotation(3);
			sprintf(tempStr1,"%s, Fmem: %lu ",System.version().c_str(), System.freeMemory() );
			displayStrings(tempStr1, 1, OCCUPANT_FONT, RED, 20 , 17, 20, 120, 12, BG_COLOR );
			//	delay(1000);
			tft.setRotation(0);	tft.setTextColor(OCCUPANT_COLOR);  //Return to normal
		#endif

		if (icstatus >= 1)
		{
			resets++;
					//Wire.begin();    //See if the bug is fixed
		}  // Attempt to reset I2C if transmission fails.

		//	displayTime();
		Wire.beginTransmission(TEMP_SENSOR);
		icstatus = (Wire.endTransmission());
		delay(40);
		Wire.requestFrom(TEMP_SENSOR, 4);
		uint8_t b = Wire.read();
			Serial.print("\nI2C Status bits are ");
			Serial.println(b >> 6);
			//SIM int humidity = 7000;  //SIM
		int humidity = (b << 8) & 0x3f00;
		humidity |= Wire.read();
		#ifdef I2C_DEBUG
  		tft.setRotation(3);
  		tft.setFont(OCCUPANT_FONT);
  		tft.setCursor(320-160, 240-20);
  		tft.setTextSize(OCCUPANT_F_SCALE);
  		tft.setTextColor(OCCUPANT_COLOR);
  		tft.fillRect(320-160,240-20,60,15,BG_COLOR);
  
  		//tft.print("I2C Sb ");
  		tft.println(resets);
  		//	sprintf(stats, "I2C Sb   %u", icstatus);
  		//	tft.println(stats);
  		tft.setRotation(0);
		#endif

		int newHumidity = humidity / 163.83;
			Serial.print("Relative humidity is ");
			Serial.println(newHumidity);
		int temp = (Wire.read() << 6) & 0x3fc0;
		temp |= Wire.read() >> 2;
		temp *= 165;
		//int temp =909256;   //SIM temp reading
		float fTemp =( temp / 16382.0) - 40.0;    //16382=2^14 - 2
		fTemp = (fTemp * 1.8) + 32.0; // convert to fahrenheit

		int newTemp =  truncf(fTemp); //use trunc not round?
			Serial.print("Temperature is ");
			Serial.println(fTemp);
		if((indoorTemperature != newTemp) || (percentHumidity != newHumidity)) // Only udate display when changed
		{
			indoorTemperature = newTemp;
			percentHumidity = newHumidity;
			displayTemperature();
		}

	}
	pot = 4095 - analogRead(POT_PIN);
	pot = (int)	runningAverage(pot);
	//potreads++;

	if (1000 == wait)  //Print every 1000 trips through the loop
	{
		Serial.print("Potentiometer reading: ");
		Serial.println(pot);

		Serial.print("PIR reading: ");
		Serial.println(analogRead(PIR_PIN));Serial.println();
	
		//potreads = 0;  We get about 75 reads per 1000 trips through the loop
		//pottime = millis() - pottime;
	}
	//if (3550 < analogRead(PIR_PIN))
	//{
	//motionDetected = true;
	//lean more toward comfort than energy efficiency
	//If user has adjusted the potentiometer
	#ifdef POT_DEBUG
		char potInfo[24] = "1234 6789 1234 : 890123";

		//potmax = max(pot, potmax);
		//potmin = min(pot, potmin);
		if (lastChangedPot >= 0)
		{
			potDiff = abs(lastChangedPot-pot);
			diffMax = max(potDiff,diffMax);
		};
		tft.setRotation(3);	tft.setFont(OCCUPANT_FONT);		tft.setTextSize(OCCUPANT_F_SCALE); tft.setTextColor(OCCUPANT_COLOR);
		tft.setCursor(320-164, 240-20);
		tft.fillRect(320-164,240-20,144,15,BG_COLOR);//diffMax
		sprintf(potInfo,"%d, %d %d:%3d %d",potChgCount,lastChangedPot, pot, potDiff, diffMax );
		tft.print(potInfo); delay(50);
		//tft.print(potmax);tft.print("  ");tft.print(potmin);tft.print(" : ");			tft.println(potDiff);

	//**********************************************************************

	//pot =	roundl(runningAverage(pot));

	//**********************************************************************
	#endif
	tft.setRotation(0);
	if (abs(pot - lastChangedPot) > 64)
	{
		// Don't set temp on boot
		potChgCount++;  //Cout how many times the pot was used to adjust temp
		if (lastChangedPot >= 0)
			{
				//map 0-4095 pot range to 50-90 temperature range map(value, fromLow, fromHigh, toLow, toHigh);
				//int t = roundl(pot * (40.0/4095.0) + 50.0);
				int t = map(pot, 0, 4095,SP_LOW, SP_HIGH); // Use 91 because of truncation
				setSetPoint(t);
				Serial.print("Setting desired temp based on potentiometer to ");
				Serial.println(t);
			}
			lastChangedPot = pot;
		}
		
		
//		if ( tInfoRead >= 1) //Reset cumulative heat on if the info has been logged.
		currentSecs = Time.now();
		if ( currentSecs >= nextWeatherTime) // Time to log Data 
  	{
			 Particle.publish("gdweather_lkp");
   		nextWeatherTime += LOG_INTERVAL; 
  	}
		
		
		if ( currentSecs >= nextLogTime) // Time to log Data 
  	{
			logData();  
  		cumHeatOn = 1U;
  		prevCumHeatOn = millis();
  		//tInfoRead = 0;
			Particle.publish("add_2sheet_lkp");   // Log data then publish to sheet
			nextLogTime += LOG_INTERVAL; 
  	}
		prevHeat = isHeatOn;
		isHeatOn = (desiredSetPoint > indoorTemperature);
		if((cumHeatOn < 1) && (isHeatOn))
		{
			prevCumHeatOn = millis();
			cumHeatOn = 1U;  // Just start the heat counter
		}


		if(isHeatOn != prevHeat) //State has changed	`
		{
			displayHeatIndicator();
		}

		if(isHeatOn)
		{
			cumHeatOn += (millis()-prevCumHeatOn);
		}
		prevCumHeatOn = millis();  //Keep updating the millis so the accrual is only since the last loop when heat was on.

			//Only update when state changes

		digitalWrite(HEAT_PIN, isHeatOn);

			//just run them at the same time for now
			//isFanOn = isHeatOn;
			//digitalWrite(FAN_PIN, isFanOn);
		if(currentSecs - previousSecs >= displayUpdInterval)
		{
					// save the last time you updated the display
			previousSecs = currentSecs;
			displayTime();
			delay(1000); // wait for a second
		}

  // format your data as JSON, don't forget to escape the double quotes
  heatSecs = round(cumHeatOn / 1000U);
 
  if(digitalRead(REPAINT_SWITCH) == LOW) paintScreen();

	#ifdef LH_DEBUG
		tft.setFont(OCCUPANT_FONT);
		tft.setCursor(240-15, L_INDOOR_RECT_Y-15);
		tft.setTextSize(OCCUPANT_F_SCALE);
		tft.setTextColor(OCCUPANT_COLOR);
		tft.fillRect(240-15,L_INDOOR_RECT_Y-15,15,15,BG_COLOR);
		if(digitalRead(REPAINT_SWITCH) == LOW)
		{
			tft.println("L");
		}
		else
			{
				tft.println("H");
			}
	#endif

	if(currentSecs - prevOccSecs >= updateOccInterval)
	{
		prevOccSecs = currentSecs;
		displayOccupants();
	}
	--wait;
}


//    prog_state check = PROG;
//switch(check){
//    case PROG: printf("neg inf"); return 1;
//    case TEMPHOLD: printf("zero"); return 2;
//    case HOLD: printf("pos inf"); return 3;
//    default: printf("not special"); break;
//}

long runningAverage(int M)  //Use this to hopefully smooth out anomalous reading from the pot
{
	#define LMSIZE 12  // number of readings.  8 or so readings per second in the loop so 
										//adjust accordingly to balance accuracy and performance
	static int LM[LMSIZE]; // LastMeasurements
	static byte index = 0;
	static long sum = 0;
	static byte count = 0;

	// keep an updated sum to improve speed.
	sum -= LM[index];
	LM[index] = M;
	sum += LM[index];
	index++;
	index = index % LMSIZE;
	if (count < LMSIZE) count++;
	return sum / count;
}

void gotWeatherReply(const char *name, const char *data) 
{
  String condit = String(data);
		Serial.print("webhook return string:");
		Serial.println(condit);
//Occasionally, the webhook returns 0's for many values because of a timeout or other failure.  
//We need to capture this and either retry or bail before we set the globals and log

//OT1	 RH	 DP	FL	 MPH WD    icon  pop  dst fh fl f1 f3
//61.2,66%,50,61.2,2.2,South,cloudy,80, 50,70, 70,70,20,100,70,
	//  hf1,hf3,ws,wd,wg

    //12,13,14,15,16,thisisit
    //There are two needles
 //   found= str.find(",",found+1,6);

 //var infolog = ctemp+","+relh+","+dewp+","+feelf+","+fhigh+","+flow+","+ppop+","+dst+","+hf1+","+hf3+","+ws+","+wd+","+iconfn+","+alertd;
	  //0         1         2         3         4         5
    //012345678901234567890123456789012345678901234567890
    //18,82,14,27,49,25,0,0,28,27,0,NE,nt_clear,\0
  int  cstart = 0;
	int length = condit.indexOf(',') - cstart;  // 0,2

	weatherObsTime =  	condit.substring(cstart,length).toInt();
						        	cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;
	outdoorTemperature = condit.substring(cstart,cstart+length).toInt();
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;
	outRelH =						 condit.substring(cstart,cstart+length).toInt();
							 cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;
	outDewp =						 condit.substring(cstart,cstart+length).toInt(); 
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;
	outFeelf =					 condit.substring(cstart,cstart+length).toInt(); 
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //feelf
	windSpeed = 				 condit.substring(cstart,cstart+length).toInt(); 
  							cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //ws
		if ( length >= 4)
		 {
		 	strcpy(windDir,condit.substring(cstart,cstart+1).c_str());  //make it a 1 char direction if longer than 3
			}
		else
			{
				strcpy(windDir,condit.substring(cstart,cstart+length).c_str()); 
			}
   							cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //wd
  outCondit = 				 condit.substring(cstart,cstart+length); 
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //iconfn
	pop = 							 condit.substring(cstart,cstart+length).toInt(); 
	
	
	//kill the remaining numbers	
							  cstart = condit.indexOf('~',cstart)+1; length = condit.indexOf(',',cstart)-cstart;
								  //ppop
	isdst = 						 condit.substring(cstart,cstart+length).toInt(); 
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //dst
 	fhighTemp =					 condit.substring(cstart,cstart+length).toInt(); 
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf('~',cstart)-cstart;  //fhigh
	flowTemp =					 condit.substring(cstart,cstart+length).toInt(); 
	//Skip 4 groups
								cstart = condit.indexOf('~',cstart)+1; length = condit.indexOf('~',cstart)-cstart;
								cstart = condit.indexOf('~',cstart)+1; length = condit.indexOf('~',cstart)-cstart; 
								cstart = condit.indexOf('~',cstart)+1; length = condit.indexOf('~',cstart)-cstart; 
								cstart = condit.indexOf('~',cstart)+1; length = condit.indexOf('~',cstart)-cstart; 
	//Skip Forcast high Hour 0							
								cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //hf0
	//Grab Forecast Hour 1
  hourForec1 = 				 condit.substring(cstart,cstart+length).toInt(); 
  							cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //hf1
  							cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //skip Forecast hour 2
  hourForec3 = 				 condit.substring(cstart,cstart+length).toInt(); 
  							cstart = condit.indexOf('~',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //look for alert strings

//OT1 RH  DP FL MPH  WD  icon  pop0 1	2	 3  4	 5	6	 7	dst1 hi1 lo1 dst2 h2 l2  dst3 h3 l3  dst4 h4 l4   hf0  hf1   2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
//43.7,71%,35,42,3.7,West,mostlycloudy,50, 0,50,50,50,50,50,50,~1,51,27~1,31,20~1,35,16~1,32,11~44,43,43,42,42,42,42,43,45,47,48,49,50,50,49,49,48,42,41,38,37,36,35,34,33,32,32,31,30,29,28,28,29,29,29,30,~
//45 ,71%,36,42,6.3,West,clear,0   ,0,0,0  ,0,0  ,0 ,0,~1  , 71  ,40~1   ,51 ,27~1   ,30,20 ~1   ,35,16  ~44  ,43   ,43,42,42,42,42,42,43,44,47,48,49,49,50,49,48,48,42,41,38,37,35,35,34,33,32,32,31,30,29,28,28,29,29,29,~
	
	//  use for accasional debug sprintf(results2,"%s  ,cstart %d,len %d, %s",condit.c_str() , cstart,length, condit.substring(cstart,length+cstart).c_str());
	if (length > 20)
		{
			length = 20;  //truncate the String to the first alert or first 20 characters for display
		}
    swalert= condit.substring(cstart,cstart+length);
	//swalert= condit.substring(condit.lastIndexOf('~')+1,20).c_str();          //  this will truncate the sw alert to 20 chars or less alertd 
	//strcpy(swalert,condit.substring(condit.lastIndexOf(',')+1));
	//strcpy(swalert,outCondit.c_str());
  //strcpy(temp,outCondit.substring(0,35));


//http://icons.wxug.com/i/c/k/nt_clear.gif  We need to clean this up
	cstart = (outCondit.lastIndexOf('/')+1);
	length = outCondit.lastIndexOf('.') - cstart;  // 0,2   
  Serial.print("Icon Original: ");Serial.println(outCondit.c_str());
	//cstart = condit.indexOf(',',cstart)+1; length = condit.indexOf(',',cstart)-cstart;  //wd
  outCondit = outCondit.substring(cstart,cstart+length); 
	//condit.substring(condit.lastIndexOf('\/')+1,20).c_str(); 
	Serial.println(outCondit.c_str());
	sprintf(wIcon,"%s/%s.bmp",WEATHER_ICON_SET,outCondit.c_str() );

	if (isdst != (int) EEPROM.read(DST_STATUS_ADDR))                                      
		{
			EEPROM.update(DST_STATUS_ADDR, (uint8_t) isdst);  // Update only writes if value is different
			Time.zone(TIME_ZONE_OFFSET + isdst);
			displayTime();
		};

	// Looks redundant here.  setOutdoorTemperature(outdoorTemperature);  // something strange here fix later
	updateTempInfo();
	
	displayOutdoorTemp();
	// set the time the weather was updated here
	return;
}

void gotAppendReply(const char *name, const char *data) 
{
  String str = String(data);
  
 	Serial.print("\nAppend to Sheet Reply: ");
 	Serial.print(str); Serial.println("\n");
}
void displaySidewaysTitles()
{
	// Write Sideways Titlesyg
	tft.setRotation(3);
	tft.setFont(SIDE_TITLE_F_SCALE);
	tft.setFont(SIDE_TITLE_FONT);

	tft.setTextColor(OUTDOOR_COLOR);
	tft.setCursor(12, 0);
	tft.println("OUTDOOR");

	tft.setTextColor(INDOOR_COLOR);
	tft.setCursor(120, 0);
	tft.println("INDOOR");

	tft.setTextColor(SETPOINT_COLOR);
	tft.setCursor(210, 0);
	tft.println("SETPOINT");
}

void displayHeatIndicator()
{
	//tft.setRotation(0);
	tft.setTextSize(HEAT_F_SCALE);
	tft.setFont(HEAT_FONT);
	if(isHeatOn){tft.setTextColor(HEAT_ON_COLOR);}
		else {tft.setTextColor(HEAT_OFF_COLOR);}
	tft.fillRect(HEAT_DISPLAY_X,HEAT_DISPLAY_Y,14,20,BG_COLOR);
	tft.setCursor(HEAT_DISPLAY_X, HEAT_DISPLAY_Y);
	tft.println("O");
}

void displayOccupants()
{
	numHome = 0;
	if(WiFi.ready() == 0)
		{
			//digitalWrite(D7,HIGH);
			//delay(1500);  let's see if it works OK without the delay
		}
		Serial.println("Checking Occupants");
		
	tft.setFont(OCCUPANT_FONT);
	tft.setCursor(OCCUPANTS_X, OCCUPANTS_Y);
	tft.setTextSize(OCCUPANT_F_SCALE);
	tft.setTextColor(OCCUPANT_COLOR);
	tft.fillRect(OCCUPANTS_X,OCCUPANTS_Y,15,15,BG_COLOR);

	for (int i=0; i < MAX_OCCUPANTS; i++)
	{

		IPAddress remoteIP{Occupants[i][0],Occupants[i][1],Occupants[i][2],Occupants[i][3]};
		//	tft.fillRect(25,310,200,8,BG_COLOR); tft.setCursor(25,310); tft.setFont(OCCUPANT_FONT); tft.print(remoteIP);tft.print(" - ");tft.print(Occupants[i][4]); tft.println(char(Occupants[i][5]));
		Occupants[i][OCC_INDEX] = WiFi.ping(remoteIP,2);

		if(Occupants[i][OCC_INDEX] > 0)
			{
					tft.setTextColor(OCCUPANT_COLOR);
					tft.print(char(Occupants[i][OCC_INITIAL]));
					numHome++;
			}
			else if( Occupants[i][OCC_INDEX] == 0)
			{
					tft.setTextColor(ILI9341_BLUE);
					tft.print(char(Occupants[i][OCC_INITIAL]));
//				tft.print(SPACE);
			}
			else
				{
					tft.setTextColor(ILI9341_RED);
					tft.print(char(Occupants[i][OCC_INITIAL]));

				} //not home

		if (i == 1) // End of first line
			{
				tft.println();
				tft.setCursor(OCCUPANTS_X,OCCUPANTS_Y + 8);
			}
		if(i == 3)	tft.println();  // End of second line
	}
	if(numHome < 1)
	{
	    analogWrite(DISP_BACKLIGHT,DISP_BRIGHT_DIM);
	    }
	    else
	    {
	        analogWrite(DISP_BACKLIGHT,DISP_BRIGHT_MED);
	};  // Dim the display if no one is home
//digitalWrite(D7,LOW);  //What is this for?  Where did it come from?
}

void displayTime()
{
	int cAdj=0;
	//"%a %b %d, %I:%M %p"
	timStr = Time.format(Time.now(), "%a %b %e,  %l:%M %p");
	sprintf(tempStr,"%s",timStr.c_str() );
	cAdj = ((((timStr.length()-5)*8)+38)/2) -6;
	displayStrings(tempStr, TIME_F_SCALE, TIME_FONT, TIME_COLOR, 120-cAdj , TIME_DISPLAY_Y, TIME_DISPLAY_X, TIME_DISPLAY_WIDTH, DISP_HT_MSSS_14_FONT, BG_COLOR );
}

//PHOTON	sFLASH_EraseSector(DST_STATUS_ADDR);
//EEPROM.write(DST_STATUS_ADDR, (uint8_t)desiredSetPoint);
//desiredSetPoint = EEPROM.read(DESIRED_TEMP_EEPROM_ADDR);

//// PHOTON	sFLASH_EraseSector(DESIRED_TEMP_FLASH_ADDRESS);
//  int addr = DESIRED_TEMP_EEPROM_ADDR;
//	#ifdef USE_SERIAL_OUTPUT   //Allow Serial Output
//	Serial.println("Saving SetPoint to flash");
//	#endif
	//uint8_t values[2] = { (uint8_t)desiredSetPoint, 0 };
//	EEPROM.write(addr, (uint8_t)desiredSetPoint);
// PHOTON	sFLASH_WriteBuffer(values, DESIRED_TEMP_FLASH_ADDRESS, 2);
//

void displaySetPoint()
{
	//tft.setRotation(0);
  displayProgInfo();
	sprintf(tempStr,"%d:",desiredSetPoint);//	tft.print(tempStr);
	displayStrings(tempStr, LARGE_TEMP_F_SCALE, LARGE_TEMP_FONT, SETPOINT_COLOR, L_XINDENT_RECT_X , L_SETPOINT_RECT_Y, L_XINDENT_RECT_X, L_TDISP_WIDTH, L_TDISP_HEIGHT, BG_COLOR );
}

void displayProgInfo()
{
	tft.setTextSize(1);	tft.setFont(MSFTSS_14);
	tft.setTextColor(SETPOINT_COLOR);

	tft.setCursor(PROG_DISPP_X,PROG_DISP_Y);	tft.print("PROG");
	tft.setCursor(PROG_DISP_TEMP_X,PROG_DISP_Y); 	tft.print("58");
	tft.setCursor(PROG_DISP_TIME_X,PROG_DISP_Y); 	tft.println("9:00 PM");
}

void saveSetPoint()
{
		Serial.println("Saving SetPoint to flash");

	EEPROM.update(DESIRED_TEMP_EEPROM_ADDR, (uint8_t)desiredSetPoint);
}

void loadSetPoint()
{
	Serial.println("Loading and displaying SetPoint from flash");
	desiredSetPoint = EEPROM.read(DESIRED_TEMP_EEPROM_ADDR);
	setSetPoint(desiredSetPoint);  //Use this to double check the value
	displaySetPoint();
}

int setSetPoint(int t)
{
	if((t < SP_LOW ) || (t > SP_HIGH))  //if an external setting wants to be out of range, go with the pot setting
		{
			pot = 4095 - analogRead(POT_PIN);
			t = map(pot, 0, 4095,SP_LOW, SP_HIGH);
		}
	desiredSetPoint = t;
	displaySetPoint();
	saveSetPoint();
	updateTempInfo();
	return desiredSetPoint;
}

int setSetPointFromString(String t)
{
	// TODO more robust error handling
	//      what if t is not a number
	//      what if t is outside 50-90 range
		Serial.print("Setting SetPoint temp from web to ");
		Serial.println(t);
	return setSetPoint(t.toInt());
}

void displayTemperature()
{
	sprintf(tempStr,"%d:",indoorTemperature);
	displayStrings(tempStr, LARGE_TEMP_F_SCALE, LARGE_TEMP_FONT, INDOOR_COLOR, L_XINDENT_RECT_X , L_INDOOR_RECT_Y, L_XINDENT_RECT_X, L_TDISP_WIDTH, L_TDISP_HEIGHT, BG_COLOR);
	sprintf(tempStr,"%d%%",int (percentHumidity));
	displayStrings(tempStr, HUMIDITY_FONT_SCALE, HUMIDITY_FONT, INDOOR_COLOR, IN_HUMIDITY_RECT_X , IN_HUMIDITY_RECT_Y, IN_HUMIDITY_RECT_X, HUMDISP_WIDTH, HUMDISP_HEIGHT, BG_COLOR );
}

void displayOutdoorTemp()
{
	int iadj = 0;

	sprintf(wIcon,"%s/%s.bmp",WEATHER_ICON_SET,outCondit.c_str() );
	// Print Alerts
	strcpy(tempStr,swalert);
		//Clear the alert first
   tft.fillRect(O_OUTDOOR_INDENT_RECT_X,L_ALERT_RECT_Y,O_OUTDOOR_DISP_WIDTH,DISP_HT_GLCD_FONT,BG_COLOR); // Clear the area first in case the alert ended
   if ( swalert != "")	{
		 	displayStrings(tempStr, 1, OCCUPANT_FONT, ORANGE, O_OUTDOOR_INDENT_RECT_X , L_ALERT_RECT_Y,O_OUTDOOR_INDENT_RECT_X, O_OUTDOOR_DISP_WIDTH, DISP_HT_GLCD_FONT, BG_COLOR );
 		};
		//Print HI and Lo
		strcpy(tempStr,"  |  ");
		sprintf(tempStr1, "%d", fhighTemp);
		displayStrings(tempStr1, tempStr, 1, HILO_FONT, FHI_COLOR, WHITE, O_OUTDOOR_CTR_X -24 , L_HILO_RECT_Y,O_OUTDOOR_INDENT_RECT_X, O_OUTDOOR_DISP_WIDTH, DISP_HT_MSSS_14_FONT, BG_COLOR, FALSE );
		//Print the second number
		sprintf(tempStr, "%d", flowTemp); tft.setTextColor(DARKCYAN); tft.println(tempStr);


	//Print Outdoor Temp
	// Adjust spacing for centering
	if((outdoorTemperature < 0 ) || ( outdoorTemperature > 99))
		{ iadj = (WEATHER_TEMP_AVG_WIDTH/2 * WEATHER_TEMP_F_SCALE) * -1;
		}
		 if((outdoorTemperature > -10) && (outdoorTemperature < 10))
			{ iadj =WEATHER_TEMP_AVG_WIDTH/2 * WEATHER_TEMP_F_SCALE;
		  }
	sprintf(tempStr, "%d%s", outdoorTemperature,":");
	displayStrings(tempStr, 1, WEATHER_TEMP_FONT, OUTDOOR_COLOR, O_OUTDOOR_INDENT_RECT_X+30 + iadj , L_OTEMP_RECT_Y,O_OUTDOOR_INDENT_RECT_X, O_OUTDOOR_DISP_WIDTH, O_OUTDOOR_DISP_HEIGHT, BG_COLOR );

	//Print Outdoor Relative Humidity
	sprintf(tempStr, "%d%%", outRelH );
	displayStrings(tempStr, 1, O_HUMIDITY_FONT, O_HUMIDITY_COLOR,  O_HUMIDITY_RECT_X , O_HUMIDITY_RECT_Y, O_HUMIDITY_RECT_X , O_HUMIDITY_WIDTH, DISP_HT_MSSS_14_FONT, BG_COLOR );

	//Print Real Feel/ Wind Chill
	sprintf(tempStr," %d~",outFeelf);//	tft.print(tempStr);
	displayStrings((char*) "Feels Like:", tempStr, 1, FEELS_FONT, FEELS_COLOR, FEELS_COLOR_H, REALFEEL_RECT_X + 15, L_REALFEEL_RECT_Y, O_OUTDOOR_INDENT_RECT_X, O_OUTDOOR_DISP_WIDTH, DISP_HT_TAHOMA_FONT, BG_COLOR, TRUE );

//Print Wind

	sprintf(tempStr," %s %d ",windDir,windSpeed);  
	//displayStrings(tempStr, 1,WIND_FONT, WIND_COLOR,  WIND_RECT_X , WIND_RECT_Y, WIND_RECT_X , WIND_WIDTH, WIND_HEIGHT, BG_COLOR );
	displayStrings((char*) "Wind", tempStr, 1, WIND_FONT, WIND_COLOR, WIND_COLOR_HL, WIND_RECT_X , WIND_RECT_Y,WIND_RECT_X, WIND_WIDTH, WIND_HEIGHT, BG_COLOR, FALSE );
	tft.setTextColor(WIND_COLOR);tft.println("MPH");
	//Print Precipitation
	sprintf(tempStr, " %d%%", pop );
	displayStrings((char*) "Precip:", tempStr, 1, PRECIP_FONT, PRECIP_COLOR, DARKORANGE, L_PRECIP_RECT_X , L_PRECIP_RECT_Y, L_PRECIP_RECT_X, PRECIP_RECT_WIDTH, DISP_HT_MSSS_14_FONT, BG_COLOR, TRUE );

	//Load the Weather conditions Icon
	tft.fillRect(WCONDITIONS_X,WCONDITIONS_Y,W_ICON_RECT_SIZE,W_ICON_RECT_SIZE,BG_COLOR);
  bmpDraw(wIcon , WCONDITIONS_X, WCONDITIONS_Y);
}

void displayStrings(char *prnStr1,char *prnStr2, int texSize, int dFont, int tColorP, int iColorA, int16_t rectX , int16_t rectY, int16_t fillRectX, int16_t rectWid, int16_t fontHt, uint16_t bgColor, bool endline )
{
	//Provide 2 Strings, 2 colors and coordinates. The second X coordinate is for the background fill which may be different than the Text
	tft.setTextSize(texSize);
	tft.setFont(dFont);
	tft.setTextColor(tColorP);
	tft.setCursor(rectX,rectY);
	tft.fillRect(fillRectX,rectY,rectWid,fontHt,bgColor);  //here
	tft.print(prnStr1);tft.setTextColor(iColorA);
	if (endline)
		{
			tft.println(prnStr2);
			}
			else
				{ tft.print(prnStr2);
					}
	}

void displayStrings(char *prnStr1, int texSize, int dFont, int tColorP, int16_t rectX , int16_t rectY, int16_t fillRectX, int16_t rectWid, int16_t fontHt, uint16_t bgColor)
{
	//Provide 1 String, 1 color and coordinates  The second X coordinate is for the background fill which may be different than the Text
	tft.setTextSize(texSize);
	tft.setFont(dFont);
	tft.setTextColor(tColorP);
	tft.setCursor(rectX,rectY);
	tft.fillRect(fillRectX,rectY,rectWid,fontHt,bgColor);  //here
	tft.println(prnStr1);
	}

int setOutdoorTemperature(int o)
{
	outdoorTemperature = o;
	//displayOutdoorTemp();
	//saveSetPoint();
	return outdoorTemperature;
}


//void gotWeatherReply(const char *name, const char *data) {
  //  String str = String(data);
    // We're really just processing a return code
  //  }
  //int setConditsFromString(String condit)
  

int setForecastFromString(String forecast)  //Deprecated?
{
	Serial.print("Setting Forecast from web to ");
	Serial.println(forecast);
	fhighTemp = forecast.substring(0,forecast.indexOf(',')).toInt();
	flowTemp = forecast.substring(forecast.indexOf(',')+1).toInt();
	displayOutdoorTemp();
	//return setOutdoorTemperature(forecast.toInt());
	return fhighTemp;
}


    
      
int settInfoReadFromString(String o)
{
	Serial.print("Setting tempInfo temp from web to ");
	Serial.println(o);
	return setInfoTemp(o.toInt());
}

int setInfoTemp(int o)
{
	tInfoRead = o;
	return tInfoRead;
}

void paintScreen()
{
	//tft.begin();

	tft.fillScreen(BG_COLOR);
	tft.setCursor(0, 0);

	displaySidewaysTitles();
	tft.setRotation(0);
	displayTime();
	displaySetPoint();
	displayTemperature();
	// Insert call here to get forecast and outdoor temp loaded before display or log
	displayOutdoorTemp();
	displayHeatIndicator();
	displayOccupants();
	digitalWrite(REPAINT_SWITCH,HIGH);
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on sd card
  if ((bmpFile = sd.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line,  this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void updateTempInfo()
{
		sheetStamp = (double(currentSecs) / dailySeconds) + float(dayConv); // Seconds per day  (A1/86400)+(25569)const float dailySeconds = 86400.0; 
  	sprintf(resultstr, "{\"spt\":%d,\"int\":%d,\"inh\":%d,\"out\":%d,\"off\":%d,\"hos\":%d,\"orh\":%d,\"dew\":%d,\"efl\":%d,\"frr\":%s,\"fon\":%d,\"hon\":%d,\"hdn\":%d,\"hot\":%d,\"crs\":%u,\"stp\":%15.7f,\"fht\":%d,\"flt\":%d,\"pop\":%d,\"hf1\":%d,\"hf3\":%d,\"nhm\":%d,\"pst\":%d,\"dst\":%d,\"frr\":%8.5f,\"wot\":%u}",
		desiredSetPoint, indoorTemperature, int(percentHumidity),outdoorTemperature,outFeelf, heatSecs,outRelH,outDewp ,estFuelLevel, pheat, isFanOn, isHeatOn, isHoldOn, holdTemp, currentSecs, sheetStamp,  fhighTemp, flowTemp, pop, hourForec1, hourForec3,  numHome, pState, isdst, fuelRunRate ,weatherObsTime); //pState
		Serial.println(resultstr);
		return;
};

void logData()
{
	updateTempInfo();

   RGB.control(true);
   RGB.color(138,43,026);

  sprintf(dataString, "%u,%15.7f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%8.5f,%u",
         	currentSecs, sheetStamp, desiredSetPoint, indoorTemperature, int(percentHumidity), outdoorTemperature,outFeelf, heatSecs, outRelH, outDewp,
         	fhighTemp, flowTemp, pop, hourForec1, hourForec3, estFuelLevel, numHome, pState, isdst, fuelRunRate, weatherObsTime);

 // Create or open the file.

//
  File dataFile = sd.open("templog.csv", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt"); 
    //    errorHalt("open failed");     //Maybe it's not the end of the world if logging fails
  }
  //int(currentSecs % LOG_INTERVAL); //Do this to see why time is wrong
  RGB.control(false);
}
   
// Print a data record.
void printData(Print* pr, data_t* data)
	{
  	pr->print(data->time);
  	for (int i = 0; i < ADC_DIM; i++)
  	{
    	pr->write(',');
    	pr->print(data->adc[i]);
  	}
  	pr->write(',');
	pr->print(data->efrr);
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  pr->println(F("time,SetPoint,InTemp,pctHum,OutTemp,OutFeel,heatsecs,outRelH,outDewp,fhi,flo,pop,hf1,hf3,EstFuel,numome,pgm,isDST,FuelRunRate"));
}


