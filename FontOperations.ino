/*
	methods for drawing GLCD fonts are defined here
	Fonts are created using GLCDFontCreator2
*/

struct	{
	uint16_t _txtClr;
	int16_t _cursorX, _cursorY;
	const uint8_t *_font;
	uint8_t _fontW, _fontH;
	uint8_t _firstCharInFont, _numCharsInFont, _nDataLines;
	uint8_t _charSpacing, _lineSpacing;
} 
fontData;


// ------------------------
void setTextColor(uint16_t txtClr)	{
// ------------------------
	fontData._txtClr = txtClr;
}


// ------------------------
void setCursor(int16_t cursorX, int16_t cursorY) {
// ------------------------
	fontData._cursorX = cursorX;
	fontData._cursorY = cursorY;
}


// ------------------------
void setCursorX(int16_t cursorX) {
// ------------------------
	fontData._cursorX = cursorX;
}


// ------------------------
void setCursorY(int16_t cursorY) {
// ------------------------
	fontData._cursorY = cursorY;
}


// ------------------------
void setFont(const uint8_t *font)	{
// ------------------------
	fontData._font = font;
	
	// read width, height into variables
	fontData._fontW = pgm_read_byte_near(fontData._font + 2);
	fontData._fontH = pgm_read_byte_near(fontData._font + 3);
	fontData._firstCharInFont = pgm_read_byte_near(fontData._font + 4);
	fontData._numCharsInFont = pgm_read_byte_near(fontData._font + 5);
	
	fontData._nDataLines = fontData._fontH / 8;
	if((fontData._nDataLines * 8) < fontData._fontH)
		fontData._nDataLines++;
	
	fontData._charSpacing = fontData._nDataLines;
	fontData._lineSpacing = fontData._nDataLines - 1;
}


// ------------------------
uint8_t drawChar(char ch)	{
// ------------------------
	// is character available
	uint8_t charIndex = ch - fontData._firstCharInFont;
	
	if( (charIndex >= 0) && (charIndex < fontData._numCharsInFont) )	{
		// get this character size 
		uint8_t charSize = pgm_read_byte_near(fontData._font + charIndex + 6); 
		
		if(charSize == 0)
			return 0;
		
		uint8_t fontDataIndex = 6 + fontData._numCharsInFont;
		int charDataIndex = fontDataIndex;
		
		for(uint8_t i = 0; i < charIndex; i++)
			charDataIndex += pgm_read_byte_near(fontData._font + 6 + i) * fontData._nDataLines;
		
		// plot font bytes
		for(uint8_t fLine = 0; fLine < fontData._nDataLines; fLine++)	{
			uint8_t pixelsRemaining = fontData._fontH - (fLine * 8);
			// number of pixels to draw in this line
			uint8_t nPixels2Draw = (pixelsRemaining < 8) ? pixelsRemaining : 8;
			
			for(uint8_t x = 0; x < charSize; x++)	{
				uint8_t linPixels = pgm_read_byte_near(fontData._font + x + charDataIndex);
				
				for(uint8_t j = 8 - nPixels2Draw, idx = 0; j < 8; j++, idx++)
					if(bitRead(linPixels, j) == 1)
						display.drawPixel(fontData._cursorX + x, fontData._cursorY + idx + (fLine * 8), fontData._txtClr);
					
			}
			charDataIndex += charSize;
		}
		
		// return drawn character size
		return charSize;
	
	}
	else
		return 0;	
}



// ------------------------
uint8_t getWidth(char ch)	{
// ------------------------
	uint8_t charIndex = ch - fontData._firstCharInFont;
	
	if( (charIndex >= 0) && (charIndex < fontData._numCharsInFont) )	{
		// get this character size 
		return pgm_read_byte_near(fontData._font + charIndex + 6); 
	}
	else
		return 0;
}





// ------------------------
uint8_t getHeight(char ch)	{
// ------------------------
	// is character available
	uint8_t charIndex = ch - fontData._firstCharInFont;
	
	if( (charIndex >= 0) && (charIndex < fontData._numCharsInFont) )	{
		// get this character size 
		uint8_t charSize = pgm_read_byte_near(fontData._font + charIndex + 6); 
		
		if(charSize == 0)
			return 0;
		
		uint8_t fontDataIndex = 6 + fontData._numCharsInFont;
		int charDataIndex = fontDataIndex;
		
		for(uint8_t i = 0; i < charIndex; i++)
			charDataIndex += pgm_read_byte_near(fontData._font + 6 + i) * fontData._nDataLines;
		
		uint8_t orPilexArr[fontData._nDataLines - 1];
		
		for(int ln = 0; ln < fontData._nDataLines; ln++)	{
			uint8_t allPx = 0x0;
			for(int x = 0; x < charSize; x++)	{
				uint8_t dataByte = pgm_read_byte_near(fontData._font + x + charDataIndex + (ln * charSize));
				allPx = allPx | dataByte;
			}
			
			if(ln == (fontData._nDataLines - 1) )
				// does last ored pixel require lsb shift
				if((fontData._fontH % 8) != 0)
					allPx = allPx >> (8 - (fontData._fontH % 8));
			
			orPilexArr[ln] = allPx;
		}
			
		uint8_t lsbPos = findLSB(orPilexArr);
		uint8_t msbPos = findMSB(orPilexArr);
		
		return (msbPos - lsbPos + 1);
	}
	else
		return 0;	

}




// ------------------------
uint8_t getOffsetTop(char ch)	{
// ------------------------
	// is character available
	uint8_t charIndex = ch - fontData._firstCharInFont;
	
	if( (charIndex >= 0) && (charIndex < fontData._numCharsInFont) )	{
		// get this character size 
		uint8_t charSize = pgm_read_byte_near(fontData._font + charIndex + 6); 
		
		if(charSize == 0)
			return 0;
		
		uint8_t fontDataIndex = 6 + fontData._numCharsInFont;
		int charDataIndex = fontDataIndex;
		
		for(uint8_t i = 0; i < charIndex; i++)
			charDataIndex += pgm_read_byte_near(fontData._font + 6 + i) * fontData._nDataLines;
		
		uint8_t orPilexArr[fontData._nDataLines - 1];
		
		for(int ln = 0; ln < fontData._nDataLines; ln++)	{
			uint8_t allPx = 0x0;
			for(int x = 0; x < charSize; x++)	{
				uint8_t dataByte = pgm_read_byte_near(fontData._font + x + charDataIndex + (ln * charSize));
				allPx = allPx | dataByte;
			}
			
			if(ln == (fontData._nDataLines - 1) )
				// does last ored pixel require lsb shift
				if((fontData._fontH % 8) != 0)
					allPx = allPx >> (8 - (fontData._fontH % 8));
			
			orPilexArr[ln] = allPx;
		}
			
		return findLSB(orPilexArr);
	}
	else
		return 0;	

}





// ------------------------
uint8_t findLSB(uint8_t *orPilexArr)	{
// ------------------------
	uint8_t lsbPos = 0, counter = 0;
	
	// find lsb here
	for(int ln = 0; ln < fontData._nDataLines; ln++)	{
		for(int i = 0; i < 8; i++)	{
			if(bitRead(orPilexArr[ln], i) == 1)
				return counter;
			
			counter++;
		}
	}
	
	return counter;
}




// ------------------------
uint8_t findMSB(uint8_t *orPilexArr)	{
// ------------------------
	uint8_t lmsbPos = 0, counter = (fontData._nDataLines * 8) - 1;
	
	for(int ln = fontData._nDataLines - 1; ln > -1; ln--)	{
		for(int i = 7; i > -1; i--)	{
			if(bitRead(orPilexArr[ln], i) == 1)
				return counter;
			
			counter--;
		}
	}

	return counter;
}





// ------------------------
int getStringWidth(const char *str)	{
// ------------------------
	int w = 0;
	char ch;
	
	for(;;)	{
		ch = *str;
		if(ch == 0)
			break;
		
		uint8_t chWidth = getWidth(ch);
		
		// add space for displayable chars only
		if(chWidth > 0)
			w += chWidth + fontData._charSpacing;
		
		str++;
	}
	
	// reduce last inter-characters space
	if(w > fontData._charSpacing)
		w -= fontData._charSpacing;
	
	return w;
}




// ------------------------
void drawString(const char *str)	{
// ------------------------
	char ch;
	
	for(;;)	{
		ch = *str;
		if(ch == 0)
			break;
		
		fontData._cursorX += drawChar(ch) + fontData._charSpacing;
		str++;
	}
}



// ------------------------
void drawString(int16_t cursorX, int16_t cursorY, const char *str)	{
// ------------------------
	setCursor(cursorX, cursorY);
	drawString(str);
}



// ------------------------
uint8_t getFontHeight()	{
// ------------------------
	return fontData._fontH + fontData._lineSpacing;
}

