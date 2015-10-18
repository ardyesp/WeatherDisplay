// debugging macros

// #define APP_DBG

// Setup debug printing macros.
#ifdef APP_DBG
  #define DBG_PRINTER 		Serial
  #define LED1				2
  #define DBG_INIT(...) 	{ DBG_PRINTER.begin(__VA_ARGS__); 	}
  #define DBG_PRINT(...) 	{ DBG_PRINTER.print(__VA_ARGS__); 	}
  #define DBG_PRINTLN(...) 	{ DBG_PRINTER.println(__VA_ARGS__); }
#else
  #define LED1				1
  #define DBG_INIT(...) 	{}
  #define DBG_PRINT(...) 	{}
  #define DBG_PRINTLN(...) 	{}
#endif

// LED
#define LED2				2		// some modules have LED on GPIO1, other have it on GPIO2
#define LED_ON				digitalWrite(LED1, 0); digitalWrite(LED2, 0)
#define LED_OFF				digitalWrite(LED1, 1); digitalWrite(LED2, 1)

#define HOSTNAME			"WeatherThing"
#define APSSID 				"Murga_8266"

// pin defs
#define ESP_OLED_CS     15  // Pin 19, CS - Chip select
#define ESP_OLED_DC     2   // Pin 20  DC - digital signal
#define ESP_OLED_RESET  16  // Pin 15  RESET

// external switches
#define PIN_MISC			4
#define PIN_LANDSCAPE		5

// screen types
#define DISPLAY_CONSOLE		1
#define DISPLAY_LANDSCAPE	2
#define DISPLAY_PORTRAIT	3
