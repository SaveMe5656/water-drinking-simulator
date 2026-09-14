#include <graphx.h>
#include <string.h>
#include <fontlibc.h>
#include <fileioc.h>

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

char *wds_getFont(char *fontName)
{
  char *appVar;
  uint8_t *appVar_ptr = NULL;
  char *appVar_fontName;

  // loop through font pack AppVars
  while ((appVar = ti_Detect(&appVar_ptr, "FONTPACK")) != NULL)
  {
    // attempt to fetch font name
    appVar_fontName = fontlib_GetFontPackName(appVar);
    // skip if font being checked is unnamed
    if (!appVar_fontName)
      continue;
    // set font and return if name matches requested
    if (!strcmp(appVar_fontName, fontName))
      return appVar;
  }
  return NULL;
}
