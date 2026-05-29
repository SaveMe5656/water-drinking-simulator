// #include <stdlib.h>
#include <stdio.h>

#include <graphx.h>
#include "gfx/gfx.h"

#include <fontlibc.h>
#include "fonts/calc1252.h"

#include <compression.h>
#include <ti/getcsc.h>

#include "text.h"

/** useful code snippets to remember:

draw imgname as background
`zx7_Decompress(gfx_vram, imgname_compressed);`

fill screen with color from palette
`gfx_FillScreen(0);`
*/
int main()
{
	// init vars for demo
	uint8_t key;
	uint8_t bg_scene = 0, bg_flag = 1;
	uint8_t text_glyph = 0, text_style = 0, text_flag = 1;

	// get Mr. Sans font (modified Dr. Sans by DrDnar)
	char *text_font = wds_getFont("Mr. Sans");

	// set font
	fontlib_SetFont(fontlib_GetFontByIndex(text_font, text_style), 0);

	// set text drawing properties
	// fontlib_SetTransparency(true);
	fontlib_SetColors(0, 1);

	// begin graphics mode
	gfx_Begin();

	// set global palette
	gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

	// begin loop
	do
	{
		// test to draw background if flag enabled
		if (bg_flag || text_flag)
		{
			// test scene value
			switch (3 & bg_scene)
			{
				// idle if 0
			case 0:
				zx7_Decompress(gfx_vram, waterd2_compressed);
				break;

				// drinking if 1
			case 1:
				zx7_Decompress(gfx_vram, waterd33_compressed);
				break;

				// dehydrate if 2
			case 2:
				gfx_FillScreen(4);
				break;

				// overhydrate (drown) if 3 (if otherwise)
			default:
				gfx_FillScreen(5);
				break;
			}
			text_flag = 1;
		}

		// draw current glyph and debug text if flag enabled
		if (text_flag)
		{
			fontlib_SetCursorPosition(0, 0);
			fontlib_DrawGlyph(text_glyph);
			fontlib_SetCursorPosition(0, 10);
			fontlib_DrawUInt(text_glyph, 1);
		}

		// store key
		key = os_GetCSC();

		// enable draw flags
		bg_flag = 1;
		text_flag = 1;

		// test pressed key for scene switching
		switch (key)
		{
			// [right]/[up]: increment scene
		case sk_Right:
		case sk_Up:
			bg_scene++;
			break;

			// [left]/[down]: decrement scene
		case sk_Left:
		case sk_Down:
			bg_scene--;
			break;

			// default: disable bg draw flag
		default:
			bg_flag = 0;
			break;
		}

		// test pressed key for glyph printing
		switch (key)
		{
			// [+]: increment glyph code
		case sk_Add:
			text_glyph++;
			break;

		// [-]: decrement glyph code
		case sk_Sub:
			text_glyph--;
			break;

		// [0]-[9]: push digit to the end of current glyph's decimal value
		case sk_0:
			text_glyph *= 10;
			break;
		case sk_1:
		case sk_2:
		case sk_3:
		case sk_4:
		case sk_5:
		case sk_6:
		case sk_7:
		case sk_8:
		case sk_9:
			text_glyph *= 10;
			text_glyph += (key % 8 - 1) * 3 - key / 8 + 2;
			break;

		// [del]: pop last digit of current glyph's decimal value
		case sk_Del:
			text_glyph /= 10;
			break;

		// [*]: change font style
		case sk_Mul:
			text_style++;
			fontlib_SetFont(fontlib_GetFontByIndex(text_font, 1 & text_style), 0);
			break;

		// default: disable text draw flag
		default:
			text_flag = 0;
			break;
		}
	}
	// continue looping until [clear] pressed
	while (key != sk_Clear);

	// end graphics mode
	gfx_End();

	// end program
	return 0;
}
