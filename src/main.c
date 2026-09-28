// standard libraries
#include <stdio.h>
// #include <stdlib.h> // unknown if needed
#include <stdint.h> // also included by <graphx.h>, <fontlibc.h>, <ti/getcsc.h>, "data.h"
// #include <stdbool.h> // included by "data.h"; unknown if needed here
#include <time.h>

// graphics libraries
#include <graphx.h>
#include "gfx/gfx.h"

// font libraries
#include <fontlibc.h>
#include "fonts/calc1252.h"

// other TI libraries
#include <compression.h>
#include <ti/getcsc.h> // might swap out for <keypadc.h> later on
// #include <sys/timers.h> // unknown if needed

// custom libraries
#include "data.h"
#include "draw.h"

#define TICK_RATE 10        // amount of ticks/second

int main()
{
  // open data
  char *data;
  wds_openData(&data);

  // get Mr. Sans font (modified Dr. Sans by DrDnar)
  char *text_font = wds_getFont("Mr. Sans");

  // init keypress var
  uint8_t key;

  // set font
  fontlib_SetFont(fontlib_GetFontByIndex(text_font, 0), 0); // 0: default | 1: bold

  // set text drawing properties
  fontlib_SetColors(1, 0);
  fontlib_SetTransparency(true);

  // begin drawing
  wds_beginDraw();

  // set global palette
  gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

  // init gametick limiter
  clock_t clock_now,
      clock_tick_offset = 0,
      clock_tick = clock();

  // begin loop
  do
  {
    // store key
    key = os_GetCSC();

    // wait to finish tick
    do
    {
      clock_now = clock();
      // msleep(1); // delay to possibly save battery; requires <sys/timers.h>
    } while ((clock_now - clock_tick - clock_tick_offset) < (CLOCKS_PER_SEC / TICK_RATE));

    // update vars for gametick limiter
    clock_tick_offset = (CLOCKS_PER_SEC / TICK_RATE) - (clock_now - clock_tick - clock_tick_offset);
    clock_tick = clock_now;
  }
  // continue looping until [clear] pressed
  while (key != sk_Clear);

  // end drawing
  wds_endDraw();

  // close data
  wds_closeData(data);

  // end program
  return 0;
}
