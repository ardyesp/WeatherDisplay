
// weather data received from website
String websiteData;
String msgStr;

// data scraped and populated in this structure
struct WeatherInfo	{
	int curTemp;
	long currTime;
	int hi, lo;
	int rh, pop;
	int rain;
	byte icon;
	const char *msg;
	boolean error;
}
wInfo;


// ------------------------
void initStructure()	{
// ------------------------
	// initialize the variables
	wInfo.curTemp = 0;
	wInfo.currTime = 0;
	wInfo.hi = 0;
	wInfo.lo = 0;
	wInfo.rh = 0;
	wInfo.pop = 0;
	wInfo.rain = 0;
	wInfo.icon = WI_UMBRELLA;
	wInfo.msg = "Ready...";
	wInfo.error = true;
	
	// parse and format this information
	formatInfo();
}



// ------------------------
boolean getWeatherData()	{
// ------------------------
	boolean retVal;
	
	if(config.scraperOWM)
		retVal = getScrapeOWData();
	else
		retVal = getScrapeECData();
	
	// parse and format this information regardless of return value
	formatInfo();
	
	return retVal;
}


// ------------------------
boolean getScrapeOWData()	{
// ------------------------	
	wInfo.error = false;
	log("Scraping OW data");
	
	// connect to website and get current weather
	if( !getOWData(false) )	{
		wInfo.error = true;
		return false;
	}
	
	// scrape data into our struct
	wInfo.curTemp = round(extractFloat("temp"));
	log("Current temp: " + String(wInfo.curTemp));
	
	wInfo.currTime = extractInt("dt");
	log("Time: " + String(wInfo.currTime));

	// connect to website and get forecast weather
	if( !getOWData(true) )	{
		wInfo.error = true;
		return false;
	}
	
	wInfo.lo = round(extractFloat("min"));
	log("Low: " + String(wInfo.lo));

	wInfo.hi = round(extractFloat("max"));
	log("High: " + String(wInfo.hi));

	wInfo.rh = extractInt("humidity");
	log("RH: " + String(wInfo.rh));

	wInfo.rain = extractOptionalInt("rain");
	log("Prep: " + String(wInfo.rain));
	
	String tempStr;
	extractValue("description", tempStr, false);
	int index1 = tempStr.indexOf('\"');
	int index2 = tempStr.indexOf('\"', index1 + 1);
	msgStr = tempStr.substring(index1+1, index2);
	wInfo.msg = msgStr.c_str();
	log("Msg: " + msgStr);
	
	extractValue("icon", tempStr, false);
	index1 = tempStr.indexOf('\"');
	index2 = tempStr.indexOf('\"', index1 + 1);
	tempStr = tempStr.substring(index1+1, index2);
	
	wInfo.icon = mapIcon(tempStr);
	log("Icon: " + String(wInfo.icon));
	
	return !wInfo.error;
}



// ------------------------
int mapIcon(String iconStr)	{
// ------------------------
	if(iconStr == "01d")
		return WI_DAY_SUNNY;
	else if(iconStr == "02d")
		return WI_DAY_SUNNY_OVERCAST;
	else if(iconStr == "03d")
		return WI_DAY_CLOUDY;
	else if(iconStr == "04d")
		return WI_CLOUDY;
	else if(iconStr == "09d")
		return WI_SHOWERS;
	else if(iconStr == "10d")
		return WI_RAIN;
	else if(iconStr == "11d")
		return WI_THUNDERSTORM;
	else if(iconStr == "13d")
		return WI_SNOW;
	else if(iconStr == "50d")
		return WI_FOG;
	else if(iconStr == "01n")
		return WI_NIGHT_CLEAR;
	else if(iconStr == "02n")
		return WI_NIGHT_CLOUDY;
	else if(iconStr == "03n")
		return WI_NIGHT_CLOUDY;
	else if(iconStr == "04n")
		return WI_NIGHT_CLOUDY;
	else if(iconStr == "09n")
		return WI_NIGHT_SHOWERS;
	else if(iconStr == "10n")
		return WI_NIGHT_RAIN;
	else if(iconStr == "11n")
		return WI_THUNDERSTORM;
	else if(iconStr == "13n")
		return WI_NIGHT_SNOW;
	else if(iconStr == "50n")
		return WI_NIGHT_FOG;
	else
		return WI_THERMOMETER;
}



// ------------------------
float extractFloat(String fieldName)	{
// ------------------------
	String strVal;
	extractValue(fieldName, strVal, false);
	return strVal.toFloat();
}



// ------------------------
int extractInt(String fieldName)	{
// ------------------------
	String strVal;
	extractValue(fieldName, strVal, false);
	return strVal.toInt();
}



// ------------------------
int extractOptionalInt(String fieldName)	{
// ------------------------
	String strVal;
	extractValue(fieldName, strVal, true);
	return strVal.toInt();
}


// ------------------------
// get the text from : to , or }
int extractValue(String fieldName, String &dest, boolean optional)	{
// ------------------------
	return extractValue(fieldName, dest, optional, 0);
}


// ------------------------
// get the text from : to , or }
int extractValue(String fieldName, String &dest, boolean optional, int start)	{
// ------------------------
	log(" searching: " + fieldName);
	int indexField = websiteData.indexOf("\"" + fieldName + "\"", start);
	if(indexField < 0)	{
		if(!optional)
			setError(fieldName);
		dest = "-";
		return -1;
	}
	
	int index1 = websiteData.indexOf(":", indexField);
	int index2 = websiteData.indexOf(",", index1);
	int index3 = websiteData.indexOf("}", index1);

	if( (index1 < 0) || ((index2 < 0) && (index3 < 0)) )	{
		if(!optional)
			setError(fieldName);
		dest = "-";
		return -1;
	}

	if(index2 < 0)
		index2 = 9999;
	
	if(index3 < 0)
		index3 = 9999;
	
	int endIndex = (index2 < index3) ? index2 : index3;	
	
	dest = websiteData.substring(index1 + 1, endIndex);
	log(" found: " + dest);
	return endIndex;
}



// ------------------------
void setError(String fieldName)	{
// ------------------------
	log(fieldName + ": Not found");
	wInfo.error = true;
	wInfo.msg = "Parse error";
}


// ------------------------
boolean getScrapeECData()	{
// ------------------------
	wInfo.msg = "EC Scraper not done";
	wInfo.error = true;
	return false;
}


