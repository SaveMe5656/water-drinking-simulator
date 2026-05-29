// #include <stdlib.h>
#include <stdio.h>

#include <graphx.h>
#include "gfx/gfx.h"

#include <fontlibc.h>
#include "fonts/calc1252.h"

#include <compression.h>
#include <ti/getcsc.h>

#include "draw.h"
#include "text.h"

/* useful code snippets to remember:

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
	uint8_t shape_flag = 1, shape_width = 50;

	// get Mr. Sans font (modified Dr. Sans by DrDnar)
	char *text_font = wds_getFont("Mr. Sans");

	// set font
	fontlib_SetFont(fontlib_GetFontByIndex(text_font, text_style), 0);

	// set text drawing properties
	// fontlib_SetTransparency(true);
	fontlib_SetColors(0, 1);

	// begin drawing
	wds_beginDraw();

	// set global palette
	gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

	// begin loop
	do
	{
		// test to draw background if any draw flags enabled
		if (bg_flag || text_flag)
		{
			// test scene value
			switch (3 & bg_scene)
			{
				// idle if 0
			case 0:
				zx7_Decompress(gfx_vbuffer, waterd2_compressed);
				break;

				// drinking if 1
			case 1:
				zx7_Decompress(gfx_vbuffer, waterd33_compressed);
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
			shape_flag = 1;
		}

		// draw current glyph and debug text if flag enabled
		if (text_flag)
		{
			fontlib_SetCursorPosition(0, 0);
			fontlib_DrawGlyph(text_glyph);
			fontlib_SetCursorPosition(0, 10);
			fontlib_DrawUInt(text_glyph, 1);
		}

		// draw test shapes
		if (shape_flag)
		{
			// draw base shape
			gfx_SetColor(1);
			gfx_FillRectangle_NoClip(4, GFX_LCD_HEIGHT - 11, 102, 7);
			gfx_SetColor(4);
			gfx_FillRectangle_NoClip(5, GFX_LCD_HEIGHT - 10, 100, 5);

			// draw variable shape
			if (shape_width > 15)
				gfx_SetColor(2);
			else
				gfx_SetColor(5);
			gfx_FillRectangle_NoClip(5, GFX_LCD_HEIGHT - 10, shape_width, 5);
		}

		// draw frame if any draw flags enabled
		if (bg_flag || text_flag || shape_flag)
			wds_drawFrame();

		// store key
		key = os_GetCSC();

		// enable draw flags
		bg_flag = 1;
		text_flag = 1;
		shape_flag = 1;

		// test pressed key for scene switching
		switch (key)
		{
		// [up]: increment scene
		case sk_Up:
			bg_scene++;
			break;

		// [down]: decrement scene
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

		// test pressed key for shape drawing
		switch (key)
		{
		// [left]: decrease width
		case sk_Left:
			shape_width > 0 && shape_width--;
			break;

		// [right]: decrease width
		case sk_Right:
			shape_width < 100 && shape_width++;
			break;

		// default: disable shape draw flag
		default:
			shape_flag = 0;
			break;
		}
	}
	// continue looping until [clear] pressed
	while (key != sk_Clear);

	// end drawing
	wds_endDraw();

	// end program
	return 0;
}
