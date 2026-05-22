// #include <stdlib.h>

#include <graphx.h>
#include "gfx/gfx.h"

#include <compression.h>

#include <ti/getcsc.h>

int main()
{
	// init vars
	uint8_t key;
	uint8_t scene = 0;
	uint8_t flag = 1;

	// begin graphics mode
	gfx_Begin();

	// set global palette
	gfx_SetPalette(wds, sizeof_wds, 0);

	/* useful code snippets to remember

	// draw imgname as background
	zx7_Decompress(gfx_vram, imgname_compressed);

	// fill screen with color from palette
	gfx_FillScreen(0);
	*/

	// begin loop
	do
	{
		// test to draw background if draw flag enabled
		if (flag)
			// test scene value
			switch (0b11 & scene)
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

		// store key
		key = os_GetCSC();

		// enable draw flag
		flag = 1;

		// test pressed key
		switch (key)
		{
			// increment scene if [right] or [up] pressed
		case sk_Right:
		case sk_Up:
			scene++;
			break;

			// decrement scene if [left] or [down] pressed
		case sk_Left:
		case sk_Down:
			scene--;
			break;

			// disable draw flag otherwise (standard behavior)
		default:
			flag = 0;
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
