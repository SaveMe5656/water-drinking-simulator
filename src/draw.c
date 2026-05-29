#include <graphx.h>

void wds_beginDraw()
{
	// begin graphics mode
	gfx_Begin();

	// draw to graphics buffer
	gfx_SetDrawBuffer();
}

void wds_drawFrame()
{
	// swap display and graphics buffer
	gfx_SwapDraw();

	// copy display to buffer
	gfx_BlitScreen();
}

void wds_endDraw()
{
	// end graphics mode
	gfx_End();
}
