/* 
	Access Point Operation defined here
	Module will setup an AP with defined SSID and wait for client to connect to it
	After connection, client navigates to http://192.168.4.1 to configure the device wifi
	and location information
*/


const char* softAPSSID = "Murga_8266";

ESP8266WebServer *server;

// ------------------------
void setupAP()	{
// ------------------------
	log("Starting AP: " + String(softAPSSID));
	
	WiFi.mode(WIFI_AP_STA);
	WiFi.disconnect();
	delay(100);
	
	WiFi.softAP(softAPSSID);
	
	IPAddress serverIP = WiFi.softAPIP();
	char tbuf[20];
	sprintf(tbuf, "%i.%i.%i.%i", serverIP[0], serverIP[1], serverIP[2], serverIP[3]);
	log("AP IP: " + String(tbuf));
	
	// Create an instance of web server
	server = new ESP8266WebServer(80);

	// add contexts
	server->on("/", onRoot);
	server->on("/saveValues", onSave);
	server->on ( "/murga.svg", drawLogo );
	server->onNotFound([]() {
		server->send (404, "text/plain", "File Not Found\n\n" );
	});
	
	// Start the server
	server->begin();
}



// ------------------------
void scanServeAPClient()	{
// ------------------------
	server->handleClient();	
}


// ------------------------
void onRoot() {
// ------------------------

	String p1 = F("<html>\
	<body>\
		<h1>Weather Station Config</h1>\
		<img src='murga.svg' height='100' width='100' ></img>\
		<form action='saveValues' method='post'>\
		<h2>Wifi Network:</h2>\
		<table>\
			<tr>\
				<td>SSID:</td>\
				<td>\
					<select name='SSID'>");

	String p2 = F("</select>\
				</td>\
			</tr>\
			<tr><td>Password:</td><td><input type='password' name='wifiPswd'></input></td></tr>\
		</table>\
		<h2>Weather Data Source</h2>\
		<p>Data provider:\
			<input checked type='radio' name='source' value='OWN'>Open Weather Map</input>\
			<input type='radio' name='source' value='EC'>Environment Canada</input>\
			<p>City ID: <input name='cityID' required type='text'></input></p>\
			<p>Forecast City ID: <input name='fCityID' type='text'></input></p>\
		</p>\
		<p>Open Weather Map</p>");
		
	String p3 = F("<p>Realtime URL: <u>api.openweathermap.org/data/2.5/weather?id=  <b>6094817</b></u></p>\
		<p>Forecast URL: <u>http://api.openweathermap.org/data/2.5/forecast/daily?id=  <b>6094817</b></u></p>\
		<p>Environment Canada</p>\
		<p>URL: <u>http://weather.gc.ca/rss/city/  <b>bc-88_e</b>  .xml</u></p>\
		<h2>General:</h2>\
		<p>Units:\
			<input checked type='radio' name='units' value='C'>&deg;Celsius</input>\
			<input type='radio' name='units' value='F'>&deg;Fahrenheit</input>\
		</p>\
		<p>Update Interval: <input required name='interval' type='number' min='10' max='70' value='15'></input> minutes</p>\
		<div> <button type='button' onclick='location.reload()' >Reload</button> <button type='submit'>Save</button> </div>\
		</form>\
	</body>\
	</html>");

	server->send(200, "text/html", p1 + scanNetworks() + p2 + p3);
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
	String s_interval = server->arg("interval");
	String s_unit = server->arg("units");
	
	log("Read arguments: ");
	log("ssid: " + s_ssid);
	log("pwd: ******");

	log("source: " + s_source);
	log("cityID: " + s_cityid);
	log("forecast cityID: " + s_fCityid);
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
	
	s_cityid.toCharArray(config.cityID, 20);
	s_fCityid.toCharArray(config.fCityid, 20);
	
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
String p1 = F("<?xml version='1.0' standalone='no'?><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 20010904//EN' 'http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd'><svg version='1.0' xmlns='http://www.w3.org/2000/svg'\
 width='362pt' height='320pt' viewBox='0 0 362 320' preserveAspectRatio='xMidYMid meet'><g transform='translate(0,320) scale(0.100000,-0.100000)' fill='#777777' stroke='none'><path d='M2592 3170 c-145 -38 -259 -107 -369 -224 -80 -84 -134 -173 -171 \
-284 -25 -74 -26 -88 -30 -354 -2 -167 -7 -278 -13 -278 -5 0 -32 11 -58 25 -27 13 -92 37 -144 51 l-95 27 -5 52 c-6 59 -89 262 -173 424 -30 57 -54 116 -54 131 0 56 -85 150 -163 179 -44 17 -137 13 -183 -8 -43 -20 -111 -92 -120 \
-127 -4 -15 -25 -32 -59 -49 -71 -34 -125 -109 -125 -172 l0 -27 28 26 c35 33 65 48 96 48 36 0 33 -15 -19 -76 -73 -85 -90 -148 -61 -219 14 -34 92 -125 106 -125 11 0 94 143 121 208 12 29 23 51 25 49 10 -10 -31 -118 -84 -217 -33 ");

String p2 = F("-63 -75 -153 -92 -200 -29 -78 -31 -93 -28 -180 3 -83 8 -103 37 -160 18 -36 52 -85 75 -110 44 -47 94 -120 187 -275 29 -49 75 -112 103 -139 57 -58 57 -56 -19 -185 -42 -73 -48 -79 -85 -85 -25 -4 -43 -14 -50 -26 -10 -19 -21 -20 \
-260 -20 l-249 0 24 36 c26 38 31 54 16 54 -32 0 -680 -169 -665 -173 10 -4 156 -31 324 -62 168 -30 313 -57 322 -59 15 -3 14 3 -6 59 l-23 62 144 7 c78 4 149 4 156 -1 8 -4 129 -100 270 -213 l257 -205 0 -69 c0 -64 -2 -71 -32 \
-102 l-33 -34 -247 0 c-137 0 -248 3 -248 6 0 4 9 21 20 39 l20 33 -31 4 c-29 5 -32 2 -68 -69 l-37 -74 -18 51 c-16 44 -21 50 -46 50 -24 0 -30 -6 -40 -37 -7 -21 -16 -42 -20 -46 -4 -4 -20 22 -36 57 -25 57 -32 65 -56 66 -16 0 -28 \
-2 -28 -5 0 -3 23 -55 51 -115 46 -98 53 -108 72 -101 13 5 27 25 35 49 7 23 17 42 21 42 5 0 16 -20 25 -45 24 -62 54 -62 84 0 l22 45 265 0 266 0 22 -26 c15 -17 36 -27 64 -31 37 -5 45 -2 73 26 l32 31 259 0 259 0 0 -40 0 -40 85 0 ");

String p3 = F("c78 0 85 2 85 20 0 18 -7 20 -60 20 l-60 0 0 30 0 30 65 0 c58 0 65 2 65 20 0 16 -8 19 -62 22 -57 3 -63 5 -66 26 -3 21 0 22 57 22 54 0 61 2 61 20 0 18 -7 20 -85 20 l-85 0 0 -50 0 -50 -258 0 -257 0 -28 30 c-16 17 -33 30 -38 30 -5 \
0 -9 31 -9 69 l0 68 264 216 265 216 123 3 123 3 52 -34 52 -33 438 -5 438 -6 -28 23 c-15 12 -34 35 -42 50 -35 68 28 112 240 166 l130 33 -596 1 -596 0\
-52 -55 -53 -55 -350 0 -350 0 7 27 c11 44 73 129 133 183 31 28 140 118 242 200 l185 149 11 -26 c9 -20 12 -22 17 -9 11 29 27 17 20 -16 -10 -53 14 -24 63 75 l46 91 17 -32 17 -33 45 103 c25 56 51 104 57 106 6 2 12 -15 15 -40 3 \
-38 5 -40 12 -19 5 14 33 75 64 135 l56 111 5 -60 c5 -56 6 -58 17 -33 7 15 34 58 61 95 29 40 57 92 67 128 l18 60 68 -140 c37 -77 79 -153 93 -170 l25 -30 1 26 c0 14 -7 75 -15 136 -8 61 -11 122 -6 138 7 28 8 26 35 -38 16 -36 ");

String p4 = F("33 -64 38 -61 5 3 7 31 4 62 -6 55 -4 52 33 -55 43 -123 78 -193 97 -193 8 0 8 16 -3 63 -25 104 -11 225 58 501 94 376 95 420 10 591 -101 205 -247 296 -470 294 -42 0 -109 -9 -149 -19z m-3 -149 c-157 -50 -275 -167 -409 -404 -18 \
-32 -34 -56 -37 -54 -9 10 38 172 68 234 72 150 215 241 379 242 l55 0 -56 -18z m303 -167 c61 -31 88 -102 88 -228 0 -86 -22 -261 -34 -274 -3 -3 -6 51 -6 119 0 131 -16 222 -49 288 -24 46 -84 101 -112 101 -12 0 -17 4 -13 11 8 \
14 87 3 126 -17z m238 -91 c39 -79 38 -129 -9 -300 -23 -83 -41 -168 -42 -189 -1 -34 -2 -36 -8 -14 -9 29 -6 89 16 263 15 123 14 251 -3 292 -16 40 21 -2 46 -52z m-381 -76 c13 -12 35 -47 48 -77 27 -60 63 -232 62 -295 0 -34 -4 -27 \
-28 55 -61 208 -119 278 -222 268 l-50 -5 27 28 c58 61 115 70 163 26z m-1179 -1521 l32 -84 -45 -96 c-42 -88 -48 -96 -74 -96 -16 0 -33 -4 -38 -9 -12 -11 -145 -26 -145 -17 0 6 72 119 166 262 30 45 54 88 54 96 0 12 8 28 15 28 1 0 ");

String p5 = F("17 -38 35 -84z m395 -385 c-5 -10 -378 -301 -402 -315 -10 -6 -13 28 -13 153 l0 160 28 4 c59 9 393 7 387 -2z m-485 -166 c0 -91 -2 -165 -5 -165 -7 0 -415 320 -415 326 0 2 95 4 210 4 l210 0 0 -165z'/>\
<path d='M1124 3160 c-26 -10 -72 -64 -80 -93 -3 -12 -19 -29 -37 -37 -41 -20 -67 -58 -74 -108 -8 -57 2 -87 33 -101 15 -6 27 -10 28 -9 1 2 14 20 29 40 41 56 86 86 155 103 57 15 67 15 124 0 34 -8 77 -27 96 -41 31 -25 86 -91 95 \
-117 3 -8 14 -3 32 13 74 68 94 190 31 190 -21 0 -29 9 -47 50 -27 64 -66 88 -110 69 -16 -6 -34 -19 -39 -28 -8 -14 -13 -12 -42 15 -61 58 -134 79 -194 54z'/></g></svg>");


	server->send(200, "image/svg+xml", p1+p2+p3+p4+p5);
	
}

