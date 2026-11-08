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
// | 2   | `2`   | redraw   |
// | 3   | `4`   |          |
// | 4   | `8`   |          |
// | 5   | `16`  |          |
// | 6   | `32`  |          |
// | 7   | `64`  |          |
// | 8   | `128` |          |
uint8_t flags = 1;

gfx_sprite_t *score_bg;     // background behind header text
gfx_sprite_t *hydration_bg; // background behind hydration bar
uint8_t hydration_bg_ypos;  // y-position to redraw background behind hydration bar
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
  // get font
  char *text_font = wds_getFont("Dr. Aller");

  // set font
  fontlib_SetFont(fontlib_GetFontByIndex(text_font, 0), 0); // 0: default | 1: bold

  // set text drawing properties
  fontlib_SetColors(1, 0);
  fontlib_SetTransparency(true);
  fontlib_SetWindow(5, 5, GFX_LCD_WIDTH - 5, GFX_LCD_HEIGHT - 5);
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

  uint8_t text_font_height = fontlib_GetCurrentFontHeight(),
          text_font_lineSpaceAbove = fontlib_GetSpaceAbove(),
          text_font_lineSpaceBelow = fontlib_GetSpaceBelow();
  uint8_t text_font_lineSpacing = text_font_lineSpaceAbove + text_font_lineSpaceBelow;

  score_bg = gfx_MallocSprite(
      /* GFX_LCD_WIDTH - 10 /* <- exceeds uint8_t! */ ~0 /* <- just forcing the max */,
      (2 * text_font_height) + text_font_lineSpacing);
  hydration_bg = gfx_MallocSprite(
      102,
      text_font_height + text_font_lineSpaceBelow + HYDRATION_BAR_HEIGHT);
  hydration_bg_ypos = GFX_LCD_HEIGHT - hydration_bg->height - 5;

  clock_t clock_tick = clock(), // gametick limiter; last clock tick
      clock_now;                // gametick limiter; current clock tick
  /* --------- */

  // temporary init hydration/score
  hydration = (short)(50.5 * HYDRATION_FACTOR);
  score = 0;

  // begin gameplay loop
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

      // print score value
      gfx_Sprite(score_bg, 5, 5);      // partial redraw
      fontlib_SetCursorPosition(5, 5); // position text cursor
      fontlib_DrawString("score: ");   // label
      fontlib_DrawUInt(score / 10, 1); // pre-point value
      fontlib_DrawGlyph('.');          // decimal point
      fontlib_DrawUInt(score % 10, 1); // post-point value

      // print on hydration update
      if ((hydration / HYDRATION_FACTOR) * HYDRATION_FACTOR == hydration || flags & 2)
      {
        // text value
        gfx_Sprite(hydration_bg, 5, hydration_bg_ypos);                     // partial redraw
        fontlib_SetCursorPosition(5 + text_font_height, hydration_bg_ypos); // position text cursor (not centered; too lazy to figure out how to do that efficiently)
        fontlib_DrawString("hydration: ");                                  // output label
        fontlib_DrawUInt(hydration / HYDRATION_FACTOR, 1);                  // value
        fontlib_DrawGlyph('%');                                             // percent sign
      }

      // update flags after redraw
      if (flags & 2)
        flags &= ~2;

      // increment/decrement
      hydration--;
      score++;
      /* -------------- */

      // print screen
      wds_drawFrame();

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
  gfx_GetSprite(hydration_bg, 5, hydration_bg_ypos);

  // set redraw flag
  flags |= 2;
}
