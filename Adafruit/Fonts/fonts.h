//
//  fonts.h
//

#ifndef _fonts_h
#define _fonts_h

#include "application.h"

// Font selection descriptors - Add an entry for each new font and number sequentially
												//AvWid	MaxW	Height	CapH		Notes
#define TIMESNR_8		0		 // 6		11 		12
#define CENTURY_8		1		 // 5		 8 		11
#define ARIAL_8			2 	 // 5		10 		10
#define COMICS_8		3 	 // 6		12 		11
#define GLCDFONT		4		// 	5		 8 		 8				6
#define TEST				5		// 10		17 		15
#define LCONSOLE_68 6		// 45		49 		84
#define TAHOMA_12		7		//  9		14 		16
#define CALIBRI_36  8		// 17		20		33					(- is 11 wide) Numbers and Symbols only : is Degree
#define CALIBRI_72  9		// 37		44		82			69?	(- is 23 wide) (% is 62 wide) (Deg is 25) 44+44+25=113 23+113=136 Max Disp
#define MSFTSS_14   10	//	8		17		20      16  high for non-descenders
#define MSFTSS_12   11	//	8		15		17      16  high for non-descenders
#define MSFTSS_26   12	// 16		31		35					27+34=  61 Humid Max

#define FONT_START 0
#define FONT_END 1


struct FontDescriptor
{
	uint8_t	width;		// width in bits
	uint8_t	height; 	// char height in bits
	uint16_t offset;	// offset of char into char array
};

// Font references - add pair of references for each new font
//extern const uint8_t timesNewRoman_8ptBitmaps[];
//extern const FontDescriptor timesNewRoman_8ptDescriptors[];

//extern const uint8_t centuryGothic_8ptBitmaps[];
//extern const FontDescriptor centuryGothic_8ptDescriptors[];

extern const uint8_t arial_8ptBitmaps[];
extern const FontDescriptor arial_8ptDescriptors[];

//extern const uint8_t comicSansMS_8ptBitmaps[];
//extern const FontDescriptor comicSansMS_8ptDescriptors[];

extern const uint8_t glcdfontBitmaps[];
extern const FontDescriptor glcdfontDescriptors[];

extern const uint8_t testBitmaps[];
extern const FontDescriptor testDescriptors[];

//extern const uint8_t lucidaConsole_68ptBitmaps[];
//extern const FontDescriptor lucidaConsole_68ptDescriptors[];

extern const uint8_t tahoma_12ptBitmaps[];
extern const FontDescriptor tahoma_12ptDescriptors[];

extern const uint8_t calibri_36ptBitmaps[];
extern const FontDescriptor calibri_36ptDescriptors[];

extern const uint8_t calibri_72ptBitmaps[];
extern const FontDescriptor calibri_72ptDescriptors[];

extern const uint8_t microsoftSansSerif_14ptBitmaps[];
extern const FontDescriptor microsoftSansSerif_14ptDescriptors[];

extern const uint8_t microsoftSansSerif_12ptBitmaps[];
extern const FontDescriptor microsoftSansSerif_12ptDescriptors[];

extern const uint8_t microsoftSansSerif_26ptBitmaps[];
extern const FontDescriptor microsoftSansSerif_26ptDescriptors[];


#endif
