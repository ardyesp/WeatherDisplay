/*
	Fetch data from weather service and render on screen
	ESP8266 Hardware SPI pins
		GPIO12   MISO (DIN)
		GPIO14   CLOCK          - (D0)
		GPIO13   MOSI (DOUT)    - (D1)
		GPIO15   CS
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ESP_SSD1306.h> 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Ticker.h>

// custom GLCD fonts
#include "Helv32.h"
#include "TPSS8.h"
#include "Unifont16.h"
#include "Weather32.h"
#include "Splash.h"

// pin defs
#define ESP_OLED_CS     15  // Pin 19, CS - Chip select
#define ESP_OLED_DC     2   // Pin 20  DC - digital signal
#define ESP_OLED_RESET  16  // Pin 15  RESET

// repetetive task times
#define SCREEN_FLIP_INTERVAL		5000
#define MISC_REFRESH_INTERVAL		60000

// external switches
#define PIN_LANDSCAPE		5

// screen types
#define DISPLAY_CONSOLE		1
#define DISPLAY_LANDSCAPE	2
#define DISPLAY_PORTRAIT	3


// app configuration entries
enum Scale	{
	CELSIUS, 
	FAHRENHEIT, 
};

// configuration variables
struct ConfigEntries	{
	char ssid[20], password[20];
	boolean scraperOWM = true;
	char cityID[20], fCityid[20];
	long interval;
	Scale unit = CELSIUS;
}
config;


// app variables

// OLED display (128x64) used in SPI mode
ESP_SSD1306 display(ESP_OLED_DC, ESP_OLED_RESET, ESP_OLED_CS);

// logging variables
#define MAX_LOG_LINES	8
String logLines[MAX_LOG_LINES];
int currentLogLine = 0;

// program variables
byte displayFormat = DISPLAY_CONSOLE;
boolean screen1 = false; 

static unsigned int lastDisplayMode = displayFormat;

// wifi operation model - AP or STA
boolean apModeOperation = false;
boolean firstTime = true;

// ------------------------
void setup()   {
// ------------------------
	Serial.begin(115200);
	
	// start display device
	initDisplay();
	delay(2000);
	
	// setup console show switch - pullup
	pinMode(PIN_LANDSCAPE, INPUT_PULLUP);
	
	initStructure();
	
	// load the configuration from flash memory
	if( !readConfig() )
		apModeOperation = true;
	
	if(apModeOperation)	
		setupAP();
	else
		setupSTA();

	log("Free Heap: " + String(ESP.getFreeHeap()));
}



// ------------------------
void loop() {
// ------------------------
	if(apModeOperation)	{
		scanServeAPClient();
	}
	else	{
		// scheduled tasks - flip screen, refresh weather, misc
		handleTasks();
		
		// read and process status of switches
		handleSwitches();
	}

	// reevaluate after a while
	delay(200);
}




// ------------------------
void handleTasks() {
// ------------------------
	static unsigned long last1 = 0;
	static unsigned long last2 = 0; 
	static unsigned long last3 = 0; 
	
	if ( last1 < millis() )	{
		last1 = millis() + SCREEN_FLIP_INTERVAL;
		// toggle screen 2 flag
		screen1 = !screen1;
		// invoke draw to update display
		draw();
	}

	if ( last2 < millis() )	{
		last2 = millis() + config.interval;
		// update weather information
		boolean gotWeather = getWeatherData();
		
		if(firstTime && gotWeather)	{
			firstTime = false;
			// switch the display from console to default
			displayFormat = DISPLAY_PORTRAIT;
		}
		
		// invoke draw to update display
		draw();
	}
	
	if ( last3 < millis() )	{
		last3 = millis() + MISC_REFRESH_INTERVAL;
		// debugging info, print heap memory stat
		Serial.print("Heap: ");Serial.println(ESP.getFreeHeap());
	}
}



// ------------------------
void handleSwitches() {
// ------------------------

	// read and act on tilt switch
	if( digitalRead(PIN_LANDSCAPE) )	{
		if(displayFormat == DISPLAY_PORTRAIT)	{
			displayFormat = DISPLAY_LANDSCAPE;
			draw();
		}
	}
	else	{
		if(displayFormat == DISPLAY_LANDSCAPE)	{
			displayFormat = DISPLAY_PORTRAIT;
			draw();
		}
	}
}



// ------------------------
void log(String msg)	{
// ------------------------
	// log onto serial port
	Serial.println(msg);

	// append log message to end of buffer
	logLines[currentLogLine] = msg;

	if(++currentLogLine == MAX_LOG_LINES)
		currentLogLine = 0;

	// invoke draw to update display if console visible
	if(displayFormat == DISPLAY_CONSOLE)
		draw();
}




