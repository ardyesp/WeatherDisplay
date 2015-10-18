// repetetive task times
#define SCREEN_FLIP_INTERVAL		5000
#define MISC_REFRESH_INTERVAL		60000
#define MAX_LOG_LINES				8
#define CONFIG_VALID_NUM			4297

#define AP_TIME_ON_POWERUP			65000
#define MINUTE_IN_US				(60 * 1000 * 1000u)
		
#define RTC_STORE_ADDR 				120
#define POWER_APPLIED 				120
#define RTC_WAKEUP					121

// operation state and misc info, stored between resets in RTC memory
// [0] = 123, [1] = boot reason
byte rtcStore[4];
byte *bootReason = &rtcStore[1];


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
	char apiKey[40];
	long interval;
	Scale unit = CELSIUS;
}
config;


// ------------------------
boolean readConfig()	{
// ------------------------
	EEPROM.begin(sizeof(config) + 20);

	int eeAddress = 0;

	int configValid;
	EEPROM.get(eeAddress, configValid);
	eeAddress += sizeof(int);

	if(configValid == CONFIG_VALID_NUM)	{
		EEPROM.get(eeAddress, config);
		EEPROM.end();

		log("Stored configuration: ");
		log("ssid: " + String(config.ssid));
		log("password: ******");
		log("OWN?: " + String(config.scraperOWM));
		log("cityID: " + String(config.cityID));
		log("forecast city: " + String(config.fCityid));
		log("API key: " + String(config.apiKey));
		log("interval: " + String(config.interval / (60 * 1000)));
		log("unit: " + String(config.unit));

		return true;
	}
	else
		return	false;
}


// ------------------------
void writeConfig()	{
// ------------------------
	EEPROM.begin(sizeof(config) + 20);

	int eeAddress = 0;

	int configValid = CONFIG_VALID_NUM;
	EEPROM.put(eeAddress, configValid);
	eeAddress += sizeof(int);

	EEPROM.put(eeAddress, config);

    EEPROM.commit();
	EEPROM.end();
}


// ------------------------
void writeRTC()	{
// ------------------------
	// write buffer back to RTC
	system_rtc_mem_write(RTC_STORE_ADDR, rtcStore, 4);
}


// ------------------------
void readRTC()	{
// ------------------------
	// read mem contents
	system_rtc_mem_read(RTC_STORE_ADDR, rtcStore, 4);

	if (rtcStore[0] != 123) {
		// initialize memory for first time use after reset
		rtcStore[0] = 123;
		*bootReason = POWER_APPLIED;

		writeRTC();
	}
}

