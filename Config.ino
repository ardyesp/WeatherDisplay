#define CONFIG_VALID_NUM	4296


// ------------------------
boolean readConfig()	{
// ------------------------
	EEPROM.begin(120);
	
	int eeAddress = 0;
	
	int configValid;
	EEPROM.get(eeAddress, configValid);
	eeAddress += sizeof(int);
	
	if(configValid == CONFIG_VALID_NUM)	{
		EEPROM.get(eeAddress, config.ssid);
		eeAddress += 20;
		EEPROM.get(eeAddress, config.password);
		eeAddress += 20;
		
		EEPROM.get(eeAddress, config.scraperOWM);
		eeAddress += sizeof(boolean);
		
		EEPROM.get(eeAddress, config.cityID);
		eeAddress += 20;
		
		EEPROM.get(eeAddress, config.fCityid);
		eeAddress += 20;
		
		EEPROM.get(eeAddress, config.interval);
		eeAddress += sizeof(long);
		
		EEPROM.get(eeAddress, config.unit);

		EEPROM.commit();
		EEPROM.end();
		
		log("Stored configuration: ");
		log("ssid: " + String(config.ssid));
		log("password: ******");
		log("OWN?: " + String(config.scraperOWM));
		log("cityID: " + String(config.cityID));
		log("forecast city: " + String(config.fCityid));
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
	EEPROM.begin(120);
	
	int eeAddress = 0;
	
	int configValid = CONFIG_VALID_NUM;
	EEPROM.put(eeAddress, configValid);
	eeAddress += sizeof(int);

	EEPROM.put(eeAddress, config.ssid);
	eeAddress += 20;
	EEPROM.put(eeAddress, config.password);
	eeAddress += 20;
	
	EEPROM.put(eeAddress, config.scraperOWM);
	eeAddress += sizeof(boolean);
	
	EEPROM.put(eeAddress, config.cityID);
	eeAddress += 20;
	
	EEPROM.put(eeAddress, config.fCityid);
	eeAddress += 20;
	
	EEPROM.put(eeAddress, config.interval);
	eeAddress += sizeof(long);
	
	EEPROM.put(eeAddress, config.unit);

    EEPROM.commit();
	EEPROM.end();
	
}

