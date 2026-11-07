/* libraries */
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
/* --------- */

/* macros */
#define TICK_RATE 10        // amount of ticks/second
#define COOLDOWN_START 23   // cooldown duration in ticks (10 ticks/sec * 2.28 seconds ~= 23 tick cooldown period)
#define HYDRATION_FACTOR 15 // factor between real hydration to get raw hydration value (real == raw / 15`)

#define HYDRATION_BAR_WIDTH 102
#define HYDRATION_BAR_HEIGHT 7
/* ------ */

/* global vars */
// special boolean values (one bit per value)
// | bit |  key  | property |
// | --- | ----- | -------- |
// | 1   | `1`   | drinking |
// | 2   | `2`   | print    |
// | 3   | `4`   |          |
// | 4   | `8`   |          |
// | 5   | `16`  |          |
// | 6   | `32`  |          |
// | 7   | `64`  |          |
// | 8   | `128` |          |
uint8_t flags = 1;

gfx_sprite_t *score_bg;     // background behind header text
gfx_sprite_t *hydration_bg; // background behind hydration bar
/* ----------- */

// function inits
void drawBg(uint8_t state);

// main function
int main()
{
  // open data
  char *data;
  wds_openData(&data);

  /* text properties */
  // get Mr. Sans font (modified Dr. Sans by DrDnar)
  char *text_font = wds_getFont("Mr. Sans");

  // set font
  fontlib_SetFont(fontlib_GetFontByIndex(text_font, 0), 0); // 0: default | 1: bold

  // set text drawing properties
  fontlib_SetColors(1, 0);
  fontlib_SetTransparency(true);
  /* --------------- */

  // begin drawing
  wds_beginDraw();

  // set global palette
  gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

  /* init vars */
  short hydration;                // raw hydration (real * 15)
  unsigned score;                 // score
  uint8_t saveTimer = 0;          // tick incrementer for save timer (save once every 600 ticks)
  uint8_t hydration_cooldown = 0; // hydration cooldown
  uint8_t key;                    // key handler

  score_bg = gfx_MallocSprite(
      /* GFX_LCD_WIDTH - 10 /* <- exceeds uint8_t! */ 255,
      (2 * fontlib_GetCurrentFontHeight()) + fontlib_GetSpaceAbove() + fontlib_GetSpaceBelow());
  hydration_bg = gfx_MallocSprite(102, 7);

  clock_t clock_tick = clock(), // gametick limiter; last clock tick
      clock_now;                // gametick limiter; current clock tick
  /* --------- */

  // temporary init hydration value
  hydration = (short)(50.5 * HYDRATION_FACTOR);

  // begin loop
  do
  {
    // properly hydrated
    if (1)
    {
      // store key
      key = os_GetCSC();

      /* game functions */
      // hydration
      if (!hydration_cooldown)
      {
        // test to redraw background
        if (flags & 1)
        {
          // set flag
          flags &= ~1;
          // redraw
          drawBg(0);
        }

        // hydrate on keypress
        switch (key)
        {
        case sk_Enter:
        case sk_2nd:
          // update hydration value
          hydration += 30 * HYDRATION_FACTOR;
          // init hydration cooldown
          hydration_cooldown = COOLDOWN_START;
        }
      }
      else
      {
        // decrement cooldown
        hydration_cooldown--;

        // test to redraw background
        if (!(flags & 1))
        {
          // set flag
          flags |= 1;
          // redraw
          drawBg(1);
        }
      }

      // test to update hydration status
      if ((hydration / HYDRATION_FACTOR) * HYDRATION_FACTOR == hydration || flags & 2)
      {
        // temporary hydration display
        gfx_Sprite(score_bg, 5, 5);                        // partial redraw
        fontlib_SetCursorPosition(5, 5);                   // position text cursor
        fontlib_DrawString("hydration: ");                 // output label
        fontlib_DrawUInt(hydration / HYDRATION_FACTOR, 1); // output value
        fontlib_DrawGlyph('%');                            // output value type
        // set draw flag
        flags |= 2;
      }

      // decrement hydration
      hydration--;
      /* -------------- */

      // test to print screen
      if (flags & 2)
      {
        // reset flag
        flags &= ~2;
        wds_drawFrame();
      }

      /* rate limiter */
      // wait to finish tick
      do
      {
        clock_now = clock();
        // msleep(1); // delay to possibly save battery; requires <sys/timers.h>
      } while ((clock_now - clock_tick) < (CLOCKS_PER_SEC / TICK_RATE));

      // update vars for gametick limiter
      clock_tick = clock_now;
      /* ------------ */
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

void drawBg(uint8_t state)
{
  // draw background
  zx7_Decompress(gfx_vbuffer, state ? waterd33_compressed : waterd2_compressed);

  // update partial redraw buffers
  gfx_GetSprite(score_bg, 5, 5);

  // set draw flag
  flags |= 2;
}
