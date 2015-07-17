/*
	STA mode operation defined here
	Device connects to service and gets current and forecast information periodically
*/

#define OWN_RT_HOST				"api.openweathermap.org"
#define EC_HOST					"weather.gc.ca"
#define HTTP_PORT				80
#define WEB_CONNECTION_TIMEOUT	60000


// ------------------------
void setupSTA()	{
// ------------------------
	log("Connecting to " + String(config.ssid));
	
	WiFi.disconnect();
	WiFi.mode(WIFI_STA);
	delay(100);
	
	WiFi.begin(config.ssid, config.password);

	long startTime = millis();
	
	while (WiFi.status() != WL_CONNECTED)	{
		delay(200);
		
		if( (millis() - startTime) > WEB_CONNECTION_TIMEOUT)	{
			log("Wifi connect failed");
			wInfo.error = true;
			wInfo.msg = "Wifi connection failed";
			return;
		}
	}

	IPAddress myIP = WiFi.localIP();
	char tbuf[20];
	sprintf(tbuf, "%i.%i.%i.%i", myIP[0], myIP[1], myIP[2], myIP[3]);
	log("myIP: " + String(tbuf));
}



// ------------------------
boolean getOWData(boolean forecast)	{
// ------------------------
	if(WiFi.status() != WL_CONNECTED)	{
		wInfo.error = true;
		wInfo.msg = "Wifi not connected";
		return false;
	}

	// reinitialize json storage
	websiteData = "";
	
	log("Connecting to Open weather server");
	log(" " + String(OWN_RT_HOST) + ":" + String(HTTP_PORT));

	// Use WiFiClient class to create TCP connections
	WiFiClient client;
	
	long startTime = millis();
	
	while( !client.connect(OWN_RT_HOST, HTTP_PORT) ) {
		delay(100);
		
		if( (millis() - startTime) > WEB_CONNECTION_TIMEOUT)	{
			log("Server connect failed");
			client.stop();
			return false;
		}
	}

	delay(100);
	
	if(forecast)	{
		log("Requesting forecast...");
		client.print("GET /data/2.5/forecast/daily?cnt=2&id=" + String(config.fCityid) + 
			"&units=" + ((config.unit == CELSIUS)? "metric" : "imperial") +
			" HTTP/1.1\r\n" +
			"Host: " + OWN_RT_HOST + "\r\n" + 
			"Connection: close\r\n\r\n");
	}
	else	{
		log("Requesting data...");
		client.print("GET /data/2.5/weather?id=" + String(config.cityID) + 
			"&units=" + ((config.unit == CELSIUS)? "metric" : "imperial") +
			" HTTP/1.1\r\n" +
			"Host: " + OWN_RT_HOST + "\r\n" + 
			"Connection: close\r\n\r\n");
	}
		
	client.flush();
	
	delay(1000);
	
	// Read all the lines of the reply from server into buffer
	while(client.available()) {
		websiteData += client.readStringUntil('\r');
		delay(50);
	}
	
	log("Read: " + String(websiteData.length()) + " bytes");
	
	// extract the json part from response
	int index = websiteData.indexOf("\n\n");
	
	if(index > 0)
		websiteData = websiteData.substring(index + 2);

	client.stop();
	delay(100);
	
	if(forecast)
		return postProcess();
	else
		return (websiteData.length() > 0);
}




// ------------------------
boolean postProcess()	{
// ------------------------
	String tmpStr;
	
	int dt1Index = extractValue("dt", tmpStr, true);
	if(dt1Index < 0)
		return false;

	long dt1 = tmpStr.toInt();
	log("Found dt1: " + String(dt1));
	
	int dt2Index = extractValue("dt", tmpStr, true, dt1Index);
	if(dt2Index < 0)
		return true;
	
	long dt2 = tmpStr.toInt();
	log("Found dt2: " + String(dt2));
	
	// which DT are we closer to
	if( (wInfo.currTime > dt1) && ((wInfo.currTime - dt1) > (dt2 - wInfo.currTime)) )	{
		// dt2 rules
		websiteData = websiteData.substring(dt2Index);
	}
	else	{
		websiteData = websiteData.substring(0, dt2Index);
	}

	return true;

}




