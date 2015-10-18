/*
	Fetch data from weather service and render on screen
	ESP8266 Hardware SPI pins
		GPIO12	MISO
		GPIO14	CLOCK	 - (D0 or CLK)
		GPIO13	MOSI    - (D1 or DIN)
		GPIO15	CS
		GPIO2	DC
		GPIO16	RESET
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ESP_SSD1306.h> 
#include <EEPROM.h>
#include <Ticker.h>
extern "C" {
  #include "user_interface.h"
}

// local includes
#include "global.h"
#include "config.h"
#include "webPage.h"

// custom GLCD fonts
#include "Helv32.h"
#include "TPSS8.h"
#include "Unifont16.h"
#include "Weather32.h"
#include "Splash.h"

// app variables

// OLED display (128x64) used in SPI mode
ESP_SSD1306 display(ESP_OLED_DC, ESP_OLED_RESET, ESP_OLED_CS);

// logging variables
String logLines[MAX_LOG_LINES];
int currentLogLine = 0;

// program variables
byte displayFormat = DISPLAY_CONSOLE;
boolean screen1 = false; 

static unsigned int lastDisplayMode = displayFormat;

// wifi operation model - AP+STA or just AP
boolean stayAPMode = false;
boolean staModeOperation = true;
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
	pinMode(PIN_MISC, INPUT_PULLUP);
	
	initStructure();
	
	// load the configuration from flash memory
	if( !readConfig() || !digitalRead(PIN_MISC) )
		staModeOperation = false;
	
	if(staModeOperation)
		setupSTA();
	else
		// setup AP server
		setupAP();
	
	// http server for configuring this device
	startConfigServer();

	log("Free Heap: " + String(ESP.getFreeHeap()));
}



// ------------------------
void loop() {
// ------------------------
	scanServeConfigClient();
	
	if(staModeOperation)	{
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




