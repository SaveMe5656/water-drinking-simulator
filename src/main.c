/* libraries */
// standard libraries
#include <stdint.h> // good amount of vars using stdint types
#include <time.h>   // rate limiter

// graphics libraries
#include <graphx.h>
#include "gfx/gfx.h" // graphics data

// font libraries
#include <fontlibc.h>
// #include "fonts/calc1252.h" // companion library to font

// other TI libraries
#include <compression.h> // background decompression
#include <ti/getcsc.h>   // keypress handling
#include <ti/screen.h>   // error dialog
// #include <sys/timers.h> // rate limiter battery saver

// custom libraries
#include "data.h" // wds save data handling
#include "draw.h" // wds screen draw handling
/* --------- */

/* macros */
#define TICK_RATE 10        // amount of ticks/second
#define COOLDOWN_START 23   // cooldown duration in ticks (10 ticks/sec * 2.28 seconds ~= 23 tick cooldown period)
#define HYDRATION_FACTOR 15 // factor between real hydration to get raw hydration value (real == raw / 15`)

#define TEXT_OFFSET 5 // offset from screen edges to draw text at

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

uint8_t key; // key handler

short score_xpos;           // x-position to draw score value
gfx_sprite_t *score_bg;     // background behind score value
uint8_t hydration_xpos;     // x-position to draw hydration value
uint8_t hydration_ypos;     // y-position to draw hydration value
gfx_sprite_t *hydration_bg; // background behind hydration value
/* ----------- */

// function declarations
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
  // return if font init fail
  if (text_font == NULL)
  {
    // output error message
    os_PutStrLine("Missing font AppVar!");
    os_NewLine();
    os_PutStrFull("Please send the AppVar to the calculator.");

    // await keypress
    do
    {
      key = os_GetCSC();
    } while (!key);

    return 1;
  }

  // set font
  fontlib_SetFont(fontlib_GetFontByIndex(text_font, 0), 0); // 0: default | 1: bold

  // set text drawing properties
  fontlib_SetColors(1, 0);
  fontlib_SetTransparency(true);
  fontlib_SetWindow(TEXT_OFFSET, TEXT_OFFSET, GFX_LCD_WIDTH - TEXT_OFFSET, GFX_LCD_HEIGHT - TEXT_OFFSET);
  /* --------------- */

  /* init vars */
  short hydration;                // raw hydration (real * 15)
  unsigned score;                 // score
  uint8_t saveTimer = 0;          // tick incrementer for save timer (save once every 600 ticks)
  uint8_t hydration_cooldown = 0; // hydration cooldown

  uint8_t text_font_height = fontlib_GetCurrentFontHeight(); // font height

  score_xpos = TEXT_OFFSET + fontlib_GetStringWidth("score: ");
  score_bg = gfx_MallocSprite(0x7f, text_font_height);
  hydration_xpos = TEXT_OFFSET + text_font_height + fontlib_GetStringWidth("hydration: ");
  hydration_ypos = GFX_LCD_HEIGHT - (TEXT_OFFSET + text_font_height + fontlib_GetSpaceBelow() + HYDRATION_BAR_HEIGHT);
  hydration_bg = gfx_MallocSprite(HYDRATION_BAR_WIDTH - hydration_ypos + TEXT_OFFSET, text_font_height);

  clock_t clock_tick = clock(), // gametick limiter; last clock tick
      clock_now;                // gametick limiter; current clock tick
  /* --------- */

  // temporary init hydration/score
  hydration = (short)(50.5 * HYDRATION_FACTOR);
  score = 0;

  // begin drawing
  wds_beginDraw();

  // set global palette
  gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

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
      gfx_Sprite(score_bg, score_xpos, TEXT_OFFSET);      // partial redraw
      fontlib_SetCursorPosition(score_xpos, TEXT_OFFSET); // position text cursor
      fontlib_DrawUInt(score / 10, 1);                    // pre-point value
      fontlib_DrawGlyph('.');                             // decimal point
      fontlib_DrawUInt(score % 10, 1);                    // post-point value

      // print on hydration update
      if (((hydration + (HYDRATION_FACTOR / 2)) / HYDRATION_FACTOR) * HYDRATION_FACTOR == hydration || flags & 2)
      {
        // text value
        gfx_Sprite(hydration_bg, hydration_xpos, hydration_ypos);  // partial redraw
        fontlib_SetCursorPosition(hydration_xpos, hydration_ypos); // position text cursor (not centered; too lazy to figure out how to do that efficiently)
        fontlib_DrawUInt(hydration / HYDRATION_FACTOR, 1);         // value
        fontlib_DrawGlyph('%');                                    // percent sign
      }

      // resolve redraw
      if (flags & 2)
      {
        // game value labels
        fontlib_SetCursorPosition(TEXT_OFFSET, TEXT_OFFSET);
        fontlib_DrawString("score: ");
        fontlib_SetCursorPosition(TEXT_OFFSET + text_font_height, hydration_ypos);
        fontlib_DrawString("hydration: ");

        // update flags
        flags &= ~2;
      }

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

  return 0;
}

void drawBg(uint8_t state)
{
  // draw background
  zx7_Decompress(gfx_vbuffer, state ? waterd33_compressed : waterd2_compressed);

  // update partial redraw buffers
  gfx_GetSprite(score_bg, TEXT_OFFSET, TEXT_OFFSET);
  gfx_GetSprite(hydration_bg, TEXT_OFFSET, hydration_ypos);

  // set redraw flag
  flags |= 2;
}