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

// macros
#define TICK_RATE 10      // amount of ticks/second
#define COOLDOWN_START 23 // 10 ticks/sec * 2.28 seconds ~= 23 tick cooldown period

/*
  special boolean values (one bit per value)
  | bit |  key  | property |
  | --- | ----- | -------- |
  | 1   | `1`   | redraw   |
  | 2   | `2`   | print    |
  | 3   | `4`   |          |
  | 4   | `8`   |          |
  | 5   | `16`  |          |
  | 6   | `32`  |          |
  | 7   | `64`  |          |
  | 8   | `128` |          |
*/
uint8_t flags = 1;

// function defs
void wds_redraw(uint8_t state);

// main function
int main()
{
  // open data
  char *data;
  wds_openData(&data);

  // get Mr. Sans font (modified Dr. Sans by DrDnar)
  char *text_font = wds_getFont("Mr. Sans");

  // set font
  fontlib_SetFont(fontlib_GetFontByIndex(text_font, 0), 0); // 0: default | 1: bold

  // set text drawing properties
  fontlib_SetColors(1, 0);
  fontlib_SetTransparency(true);

  // begin drawing
  wds_beginDraw();

  // set global palette
  gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

  // init vars
  unsigned score;               // raw score (real * 10)
  short hydration;              // raw hydration (real * 15)
  uint8_t key = os_GetCSC();    // key handler
  uint8_t saveTimer = 0;        // tick incrementer for save timer (save once every 600 ticks)
  clock_t clock_tick = clock(), // gametick limiter; last clock tick
      clock_now;                // gametick limiter; current clock tick
  uint8_t cooldown = 0;         // hydration cooldown

  // begin loop
  do
  {
    // properly hydrated
    if (1)
    {
      // hydration
      if (!cooldown)
      {
        // test to redraw scene
        if (flags & 1)
        {
          // set flag
          flags &= ~1;
          // redraw scene
          wds_redraw(0);
        }

        // hydrate on keypress
        switch (key)
        {
        case sk_Enter:
        case sk_2nd:
          cooldown = COOLDOWN_START;
        }
      }
      else
      {
        // decrement cooldown
        cooldown--;

        // test to redraw scene
        if (!(flags & 1))
        {
          // set flag
          flags |= 1;
          // redraw scene
          wds_redraw(1);
        }
      }

      // test to print screen
      if (flags & 2)
      {
        // reset flag
        flags &= ~2;
        wds_drawFrame();
      }

      // wait to finish tick
      do
      {
        clock_now = clock();
        // msleep(1); // delay to possibly save battery; requires <sys/timers.h>
      } while ((clock_now - clock_tick) < (CLOCKS_PER_SEC / TICK_RATE));

      // update vars for gametick limiter
      clock_tick = clock_now;

      // store key
      key = os_GetCSC();
    }
    // improperly hydrated
    else
    {
    }
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

void wds_redraw(uint8_t state)
{
  // draw background
  zx7_Decompress(gfx_vbuffer, state ? waterd33_compressed : waterd2_compressed);

  // set draw flag
  flags |= 2;
}
