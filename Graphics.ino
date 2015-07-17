// OLED screen constant
#define ROTATION_0	0
#define ROTATION_90	1

// display fonts
#define PORT_TEMP_FONT		Helv32
#define PORT_HILO_FONT		Unifont16		
#define PORT_TEXT_FONT		TPSS8			
#define LAND_TEMP_FONT		Helv32
#define LAND_HILO_FONT		Unifont16		
#define WEATHER_FONT		Weather32


struct InfoString	{
	int x, y;
	char buf[5];
};

struct BigInfoString	{
	int x, y;
	char buf[64];
};


struct FormattedInfo	{
	InfoString curTemp, hi, lo, rh, pop, rain;
	InfoString icon;
	BigInfoString msg;
	boolean scrollingRequired = false;
} 
portraitInfo, landscapeInfo;

// app variables
Ticker scroller;
int messageLength = 0;

// ------------------------
void initDisplay()	{
// ------------------------
	display.begin(SSD1306_SWITCHCAPVCC);
	display.clearDisplay();
	// built in font
	display.setTextColor(WHITE);
	display.setTextWrap(false);
	// custom fonts
	setTextColor(WHITE);
	
	// show splash screen
	display.setRotation(ROTATION_90);
	display.drawBitmap(0, 10, murga, MURGA_WIDTH, MURGA_HEIGHT, 1);

	setFont(PORT_HILO_FONT);
	const char* splashMsg1 = "Weather";
	const char* splashMsg2 = "Station";
	
	int x1 = (64 - getStringWidth(splashMsg1)) / 2;
	int x2 = (64 - getStringWidth(splashMsg2)) / 2;
	
	drawString(x1, 80, splashMsg1);
	drawString(x2, 96, splashMsg2);

	// render on screen
	display.display();	
}



// ------------------------
void draw()	{
// ------------------------
	// clear whatever is being drawn
	display.clearDisplay();

	// pick what to show
	switch(displayFormat)	{
		case DISPLAY_CONSOLE:	showConsole(); 		break;
		case DISPLAY_LANDSCAPE: drawLandscape();	break;
		case DISPLAY_PORTRAIT: 	drawPortrait();		break;
	}
	
	// send draw buffer to display
	display.display();
}


// ------------------------
void showConsole()	{
// ------------------------
	display.setRotation(ROTATION_0);
	byte lineHeight = 8;
	
	int lineNum = 0;
	
	for(int i = currentLogLine; i < MAX_LOG_LINES; i++)	{
		display.setCursor(0, lineNum++ * lineHeight);
		display.print(logLines[i]);
	}
	
	for(int i = 0; i < currentLogLine; i++)	{
		display.setCursor(0, lineNum++ * lineHeight);
		display.print(logLines[i]);
	}

	// if showing console, slow down scrolling
	delay(300);
}




// ------------------------
void drawLandscape()	{
// ------------------------
	display.setRotation(ROTATION_0);

	// main temperature
	setFont(LAND_TEMP_FONT);
	drawString(landscapeInfo.curTemp.x, landscapeInfo.curTemp.y, landscapeInfo.curTemp.buf);
	setFont(WEATHER_FONT);
	setCursorY(-10);
	if(config.unit == CELSIUS)
		drawChar(WI_CELSIUS);
	else
		drawChar(WI_FAHRENHEIT);

	// 32x32 icon
	setFont(WEATHER_FONT);
	setCursor(landscapeInfo.icon.x, landscapeInfo.icon.y);
	drawChar(wInfo.icon);
	
	// hi lo error display
	setFont(LAND_HILO_FONT);
	
	// draw error icon
	if(wInfo.error)
		drawString(0, 0, "!");
	
	drawString(70, 35, "Hi");
	drawString(70, 50, "Lo");

	drawString(landscapeInfo.hi.x, landscapeInfo.hi.y, landscapeInfo.hi.buf);
	drawString(landscapeInfo.lo.x, landscapeInfo.lo.y, landscapeInfo.lo.buf);
}



// ------------------------
void drawPortrait()	{
// ------------------------
	display.setRotation(ROTATION_90);
	
	setFont(PORT_TEMP_FONT);
	drawString(portraitInfo.curTemp.x, portraitInfo.curTemp.y, portraitInfo.curTemp.buf);
	setFont(WEATHER_FONT);
	setCursorY(-10);
	drawChar(WI_DEGREES);
	
	// partition line
	display.drawFastHLine(0, 34, display.width(), WHITE);

	// hi lo display
	setFont(PORT_HILO_FONT);
	
	if(screen1)	{
		drawString(0, 38, "Hi");
		const char *Lo = "Lo";
		int strW = getStringWidth(Lo);
		drawString(display.width() - strW, 38, Lo);
		drawString(portraitInfo.hi.x, portraitInfo.hi.y, portraitInfo.hi.buf); 
		drawString(portraitInfo.lo.x, portraitInfo.lo.y, portraitInfo.lo.buf);
	}
	else	{
		drawString(0, 38, "Rh");

		if(config.scraperOWM)	{
			int strW = getStringWidth("Rn");
			drawString(display.width() - strW, 38, "Rn");
			drawString(portraitInfo.rain.x, portraitInfo.rain.y, portraitInfo.rain.buf);
		}
		else	{
			int strW = getStringWidth("PoP");
			drawString(display.width() - strW, 38, "PoP");
			drawString(portraitInfo.pop.x, portraitInfo.pop.y, portraitInfo.pop.buf);
		}
		
		drawString(portraitInfo.rh.x, portraitInfo.rh.y, portraitInfo.rh.buf);
	}

	// draw error icon
	if(wInfo.error)
		drawString(0, 74, "!");

	// 32x32 icon
	setFont(WEATHER_FONT);
	setCursor(portraitInfo.icon.x, portraitInfo.icon.y);
	drawChar(wInfo.icon);
	
	// forecast text
	setFont(PORT_TEXT_FONT);
	drawString(portraitInfo.msg.x, portraitInfo.msg.y, portraitInfo.msg.buf);
}



// ------------------------
void scrollMessage()	{
// ------------------------
	static int waits = 0;
	
	if((displayFormat == DISPLAY_PORTRAIT) && portraitInfo.scrollingRequired)	{
		// add pause at begining and end of message 
		if(waits++ < 10)
			return;
		
		portraitInfo.msg.x--;
		
		if(portraitInfo.msg.x == (64 - messageLength - 1))
			waits = 0;
			
		if(portraitInfo.msg.x < (64 - messageLength - 1))	{
			portraitInfo.msg.x = 0;
			waits = 0;
		}
		
		draw();
	}
}



// ------------------------
void formatInfo()   {
// ------------------------
	formatPortraitInfo();
	formatLandscapeInfo();
}



// ------------------------
void formatPortraitInfo()	{
// ------------------------ 
	// format for portrait mode
	setFont(PORT_TEMP_FONT);

	sprintf(portraitInfo.curTemp.buf, "%i", wInfo.curTemp);
	int strWidth = getStringWidth(portraitInfo.curTemp.buf);

	setFont(WEATHER_FONT);
	strWidth += getWidth(WI_DEGREES);
	
	portraitInfo.curTemp.x = (64 - strWidth)/2;
	portraitInfo.curTemp.y = 0;

	setFont(PORT_HILO_FONT);
	
	sprintf(portraitInfo.hi.buf, "%i", wInfo.hi);
	portraitInfo.hi.x = 0;
	portraitInfo.hi.y = 51;

	sprintf(portraitInfo.lo.buf, "%i", wInfo.lo);
	strWidth = getStringWidth(portraitInfo.lo.buf);
	portraitInfo.lo.x = 64 - strWidth;
	portraitInfo.lo.y = 51;
	
	sprintf(portraitInfo.rh.buf, "%i%%", wInfo.rh);
	portraitInfo.rh.x = 0;
	portraitInfo.rh.y = 51;

	sprintf(portraitInfo.pop.buf, "%i%%", wInfo.pop);
	strWidth = getStringWidth(portraitInfo.pop.buf);
	portraitInfo.pop.x = 64 - strWidth;
	portraitInfo.pop.y = 51;
	
	sprintf(portraitInfo.rain.buf, "%i", wInfo.rain);
	strWidth = getStringWidth(portraitInfo.rain.buf);
	portraitInfo.rain.x = 64 - strWidth;
	portraitInfo.rain.y = 51;
	
	setFont(WEATHER_FONT);
	int charWidth = getWidth(wInfo.icon);
	int charHeight = getHeight(wInfo.icon);
	int fromTop = getOffsetTop(wInfo.icon);
	// x goes from 0 to 64. Center the icon
	portraitInfo.icon.x = (64 - charWidth) / 2;
	// y ranges from 65 <-> (128 - 16). Center vertically
	portraitInfo.icon.y = 65 + (48 - charHeight)/2 - fromTop;

	setFont(PORT_TEXT_FONT);
	sprintf(portraitInfo.msg.buf, "%s", wInfo.msg);
	strWidth = getStringWidth(portraitInfo.msg.buf);
	if(strWidth > 70)	{
		portraitInfo.msg.x = 0;
		// setup async tasks
		messageLength = strWidth;
		portraitInfo.scrollingRequired = true;
		scroller.attach_ms(100, scrollMessage);
	}
	else	{
		portraitInfo.scrollingRequired = false;
		portraitInfo.msg.x = (64 - strWidth)/2;
		// detach scroller
		scroller.detach();
	}
	
	portraitInfo.msg.y = 128 - 16;
}



// ------------------------
void formatLandscapeInfo()   {
// ------------------------
	// format for landscape mode
	sprintf(landscapeInfo.curTemp.buf, "%i", wInfo.curTemp, 0xb0);
	landscapeInfo.curTemp.x = 55;
	landscapeInfo.curTemp.y = 0;
	
	sprintf(landscapeInfo.hi.buf, "%i", wInfo.hi);
	landscapeInfo.hi.x = 95;
	landscapeInfo.hi.y = 35;

	sprintf(landscapeInfo.lo.buf, "%i", wInfo.lo);
	landscapeInfo.lo.x = 95;
	landscapeInfo.lo.y = 50;
	
	setFont(WEATHER_FONT);
	int charWidth = getWidth(wInfo.icon);
	int charHeight = getHeight(wInfo.icon);
	int fromTop = getOffsetTop(wInfo.icon);
	// x goes from 0 to 60. Center the icon
	landscapeInfo.icon.x = (60 - charWidth) / 2;
	// y ranges from 0 <-> 64. Center vertically
	landscapeInfo.icon.y = ((64 - charHeight)/2 - fromTop) + 5;
	
	// unused for now
	sprintf(landscapeInfo.rh.buf, "%i%%", wInfo.rh);
	landscapeInfo.rh.x = 0;
	landscapeInfo.rh.y = 0;

	sprintf(landscapeInfo.pop.buf, "%i%%", wInfo.pop);
	landscapeInfo.pop.x = 0;
	landscapeInfo.pop.y = 0;
	
	sprintf(landscapeInfo.rain.buf, "%i", wInfo.rain);
	landscapeInfo.rain.x = 0;
	landscapeInfo.rain.y = 0;
	
	sprintf(landscapeInfo.msg.buf, "%s", wInfo.msg);
	landscapeInfo.msg.x = 0;
	landscapeInfo.msg.y = 0;
}

