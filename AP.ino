/* 
	Access Point Operation defined here
	Module will setup an AP with defined SSID and wait for client to connect to it
	After connection, client navigates to http://192.168.4.1 to configure the device wifi
	and location information
*/


ESP8266WebServer *server;

// ------------------------
void setupAP()	{
// ------------------------
	log("Starting AP: " + String(APSSID));
	
	WiFi.disconnect();
	WiFi.mode(WIFI_AP_STA);
	delay(100);
	
	WiFi.softAP(APSSID);
	
	IPAddress serverIP = WiFi.softAPIP();
	char tbuf[20];
	sprintf(tbuf, "%i.%i.%i.%i", serverIP[0], serverIP[1], serverIP[2], serverIP[3]);
	log("AP IP: " + String(tbuf));
}



// ------------------------
void startConfigServer()	{
// ------------------------
	// Create an instance of web server
	server = new ESP8266WebServer(80);

	// add contexts
	server->on("/", onRoot);
	server->on("/saveValues", onSave);
	server->on ("/murga.svg", drawLogo );
	server->onNotFound([]() {
		server->send (404, "text/plain", "File Not Found\n\n" );
	});
	
	// Start the server
	server->begin();
	
	log("Config server started");
}



// ------------------------
void scanServeConfigClient()	{
// ------------------------
	server->handleClient();	
}


// ------------------------
void onRoot() {
// ------------------------
	// client requested homepage. Never shutdown AP mode
	stayAPMode = true;
	// get available wifi networks
	String networks = scanNetworks();
	
	int len = networks.length();
	len += strlen(PAGE_Home1);
	len += strlen(PAGE_Home2);
	
	// send multipart homepage
	server->setContentLength(len);
	server->send(200, "text/html", PAGE_Home1);
	server->sendContent(networks);
	server->sendContent(PAGE_Home2);
}


// ------------------------
void onSave() {
// ------------------------
	// verify the returned parameters
	String s_ssid = server->arg("SSID");
	String s_password = server->arg("wifiPswd");
	
	String s_source = server->arg("source");
	String s_cityid = server->arg("cityID");
	String s_fCityid = server->arg("fCityID");
	String s_apiKey = server->arg("apiKey");
	String s_interval = server->arg("interval");
	String s_unit = server->arg("units");
	
	log("Read arguments: ");
	log("ssid: " + s_ssid);
	log("pwd: ******");

	log("source: " + s_source);
	log("cityID: " + s_cityid);
	log("forecast cityID: " + s_fCityid);
	log("API key: " + s_apiKey);
	log("interval: " + s_interval);
	log("units: " + s_unit);
	
	// verify the settings
	if(s_ssid == "")	{
		server->send(500, "text/html", "<p style='color:ff0000'>Error: Rescan and select a wifi network</p>");
		return;
	}
	
	if(s_cityid == "")	{
		server->send(500, "text/html", "<p style='color:ff0000'>Error: CityID must be provided</p>");
		return;
	}
	
	if(s_fCityid == "")	
		s_fCityid = s_cityid;
	
	int interval = s_interval.toInt();
	
	if( (interval < 10) )	{
		server->send(500, "text/html", "<p style='color:ff0000'>Error: Refresh interval must be atleast 10 minutes</p>");
		return;
	}

	
	// transform into storage variables
	s_ssid.toCharArray(config.ssid, 20);
	s_password.toCharArray(config.password, 20);
	
	if(s_source == "OWN")
		config.scraperOWM = true;
	else
		config.scraperOWM = false;
	
	
	if(config.scraperOWM && (s_apiKey == ""))	{
		server->send(500, "text/html", "<p style='color:ff0000'>Error: Open Weather Network API key must be provided</p>");
		return;
	}
	
	s_cityid.toCharArray(config.cityID, 20);
	s_fCityid.toCharArray(config.fCityid, 20);
	s_apiKey.toCharArray(config.apiKey, 40);
	
	config.interval = interval * 60 * 1000;
	
	if(s_unit == "C")
		config.unit = CELSIUS;
	else
		config.unit = FAHRENHEIT;
	
	// save settings into flash
	writeConfig();
	
	// send message
	server->send(200, "text/html", "<h1>Settings saved; Device rebooting</h1>");
	
	// reboot
	delay(5 * 1000);
	ESP.reset();	
}



// ------------------------
String scanNetworks()	{
// ------------------------
	String networks = "";
	
	log("Scanning wifi...");

	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	log(" ..done");
	if (n == 0)
		log("No networks found");
	else	{
		log(String(n) + " networks found");
		for (int i = 0; i < n; ++i)	{
			// Print SSID and RSSI for each network found
			networks += "<option value='" + String(WiFi.SSID(i)) + "'>" + String(WiFi.SSID(i)) + "   (" + String(WiFi.RSSI(i)) + "db)</option>";
			log(" " + String(WiFi.SSID(i)) );
			delay(10);
		}
	}
	
	return networks;
}


// ------------------------
void drawLogo() {
// ------------------------
	server->send(200, "image/svg+xml", SVG_Logo);
}

