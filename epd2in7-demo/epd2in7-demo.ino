/**
 *  @filename   :   epd2in7-demo.ino
 *  @brief      :   2.7inch e-paper display demo
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 22 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epd2in7.h"
#include "epdpaint.h"
#include "imagedata.h"

#include <SD.h>
#include <SPI.h>

#define COLORED     0
#define UNCOLORED   1
#define SDCARD_CLK  14
#define SDCARD_MISO 2
#define SDCARD_MOSI 15
#define SDCARD_SS   13

unsigned char image[ 5808 ];
Paint         paint( image, 176, 264 ); //width should be the multiple of 8
File          text;

int  selectBtn   = 39;
int  upBtn       = 37;
int  dwnBtn      = 38;
int  currentPage = 0;
void showDirs( Paint paint )
{
	int  i    = 0;
	File root = SD.open( "/" );
	while ( true ) {
		File entry = root.openNextFile();
		if ( !entry ) {
			break;
		}
		char buf[ 25 ] = {
			0,
		};

		snprintf( buf, 25, "%s %d B", entry.name(), entry.size() );
		paint.DrawStringAt( 0, i, buf, &Font12, COLORED );
		i += 10;
	}
}

void drawPage( int page, Paint* paint )
{
	int lineLen = 25;
	int lineNum = 26;
	// File text    = SD.open( "/0.txt" );
	text.seek( lineNum * lineLen * page );
	Serial.println( "After seek" );
	char** buf = ( char** ) malloc( lineNum * sizeof( char* ) );
	for ( size_t i = 0; i < lineNum; i++ ) {
		buf[ i ]                = ( char* ) malloc( lineLen * sizeof( char ) );
		buf[ i ][ lineLen - 1 ] = ' ';
	}

	for ( int i = 0; i < lineNum; i++ ) {
		for ( int j = 0; j < lineLen; j++ ) {
			char ch = ( char ) text.read();
			if ( ch < 32 ) {
				buf[ i ][ j ] = ' ';
			} else {
				buf[ i ][ j ] = ch;
			}
		}
		// Serial.println( buf[ i ] );
	}
	for ( int i = 0; i < lineNum; i++ ) {
		paint->DrawStringAt( 0, i * 10, buf[ i ], &Font12, COLORED );
		free( buf[ i ] );
	}
	free( buf );
}

int getPage()
{
	int  i = 0;
	char buf[ 10 ];
	File pageFile = SD.open( "/0.txt.page" );
	while ( pageFile.available() && i < 10 ) {
		buf[ i ] = pageFile.read();
		i++;
	}
	buf[ i ] = '\0';
	int page = atoi( buf );
	pageFile.close();
	return page;
}

void setPage( int page )
{
	File pageFile = SD.open( "/0.txt.page" );
	char buf[ 10 ];
	int  len = snprintf( buf, 10, "%d", page );
	for ( int i = 0; i < len; i++ ) {
		pageFile.write( buf[ i ] );
	}
	pageFile.close();
}

void setup()
{
	// put your setup code here, to run once:
	Serial.begin( 115200 );
	Serial.println( "Start" );
	Epd epd;

	pinMode( selectBtn, INPUT_PULLUP );
	pinMode( upBtn, INPUT_PULLUP );
	pinMode( dwnBtn, INPUT_PULLUP );

	if ( epd.Init() != 0 ) {
		Serial.print( "e-Paper init failed" );
		return;
	}
	Serial.println( "Scr init" );

	/* This clears the SRAM of the e-paper display */
	epd.ClearFrame();
	Serial.println( "Cleared" );

	SPIClass sdSPI( VSPI );
	sdSPI.begin( SDCARD_CLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_SS );

	// unsigned char image[ 5808 ];
	// Paint         paint( image, 176, 264 ); //width should be the multiple of 8

	// paint.Clear( UNCOLORED );
	// paint.DrawStringAt( 0, 0, "START", &Font24, COLORED );
	// epd.TransmitPartialData( paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight() );
	// epd.DisplayFrame();
	// sleep(5000);

	paint.Clear( UNCOLORED );
	Serial.println( "Cleared" );
	if ( SD.begin( SDCARD_SS, sdSPI ) ) {
		if ( SD.exists( "/0.txt" ) ) {
			text = SD.open( "/0.txt" );
			Serial.println( "Exists" );
			int page    = getPage();
			currentPage = page;
			Serial.printf( "Page: %d\n", page );
			drawPage( page, &paint );
		} else {
			showDirs( paint );
		}

	} else {
		paint.DrawStringAt( 10, 0, "SD err", &Font12, UNCOLORED );
	}
	epd.TransmitPartialData( paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight() );

	/* This displays the data from the SRAM in e-Paper module */
	epd.DisplayFrame();
	/* Deep sleep */
	epd.Sleep();
}

void loop()
{
	if ( digitalRead( dwnBtn ) == LOW ) {
		Serial.println( "Down pressed" );
		currentPage++;
		Serial.println( "Increment" );

		setPage( currentPage );
		Serial.println( "Page set" );

		drawPage( currentPage, &paint );
	}
	if ( digitalRead( upBtn ) == LOW && currentPage > 0 ) {
		Serial.println( "Up pressed" );
		currentPage--;
		setPage( currentPage );
		drawPage( currentPage, &paint );
	}
	// put your main code here, to run repeatedly:
}
