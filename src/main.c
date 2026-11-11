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

#define SCORE_LABEL "score: "         // gui label prefixing score value
#define HYDRATION_LABEL "hydration: " // gui label prefixing hydration value

#define HYDRATION_BAR_WIDTH 102
#define HYDRATION_BAR_HEIGHT 7

#define hydration_real (hydration / HYDRATION_FACTOR) // real hydration

#define text_font_regular fontlib_GetFontByIndex(text_font_name, 0) // regular font
#define text_font_bold fontlib_GetFontByIndex(text_font_name, 1)    // bold font

// pseudo-function for awaiting keypress via `os_GetCSC()`
#define wds_awaitCSC() \
  do                   \
  {                    \
    key = os_GetCSC(); \
  } while (!key)

// pseudo-function for drawing centered text relative to the whole screen, above the y-axis center line
#define wds_DrawScreenCenteredStringAbove(string, lineOffset)                                                                                   \
  fontlib_SetCursorPosition(                                                                                                                    \
      (GFX_LCD_WIDTH - fontlib_GetStringWidth(string)) / 2,                                                                                     \
      (GFX_LCD_HEIGHT - ((((lineOffset * 2) + 1) * text_font_height) + ((lineOffset * 2) * text_font_spacing) + fontlib_GetSpaceBelow())) / 2); \
  fontlib_DrawString(string)

// pseudo-function for drawing centered text relative to the whole screen, below the y-axis center line
#define wds_DrawScreenCenteredStringBelow(string, lineOffset)                                                                                   \
  fontlib_SetCursorPosition(                                                                                                                    \
      (GFX_LCD_WIDTH - fontlib_GetStringWidth(string)) / 2,                                                                                     \
      (GFX_LCD_HEIGHT + ((((lineOffset * 2) + 1) * text_font_height) + ((lineOffset * 2) * text_font_spacing) + fontlib_GetSpaceAbove())) / 2); \
  fontlib_DrawString(string)
/* ------ */

/* global vars */
unsigned hydration, // raw hydration (real * 15)
    score;          // raw score (real * 10)

uint8_t hydration_cooldown; // hydration cooldown
uint8_t saveTimer;          // save timer (save once every 600 ticks)

uint8_t key; // key handler

/**
special boolean values (one bit per value)

| bit |  key  | property        |
| --- | ----- | --------------- |
| 1   | `1`   | drinking        |
| 2   | `2`   | redraw          |
| 3   | `4`   | hydration state |
| 4   | `8`   | game over draw  |
| 5   | `16`  |        -        |
| 6   | `32`  |        -        |
| 7   | `64`  |        -        |
| 8   | `128` |        -        |

```c
// retrieve
bool abc = (flags & 4); // 3rd flag
// overwrite
flags |= 32; // enable 6th flag
flags &= ~8; // disable 4th flag
```
*/
uint8_t flags;

uint8_t score_xpos;         // x-position to draw score value
gfx_sprite_t *score_bg;     // background behind score value
uint8_t hydration_xpos,     // x-position to draw hydration value
    hydration_ypos;         // y-position to draw hydration value
gfx_sprite_t *hydration_bg; // background behind hydration value
/* ----------- */

// function declarations
void wds_drawBg(uint8_t state);
void wds_initGame();

// main function
int main()
{
  // open data
  char *data;
  wds_openData(&data);

  /* text properties */
  // get font
  char *text_font_name = wds_getFont("Dr. Aller");
  // return if font init fail
  if (text_font_name == NULL)
  {
    // output error message
    os_PutStrLine("Missing font AppVar!");
    os_NewLine();
    os_PutStrFull("Please send the AppVar to the calculator.");

    // await keypress
    wds_awaitCSC();

    return 1;
  }

  // set text drawing properties
  fontlib_SetFont(text_font_regular, 0);                                                                  // use regular font
  fontlib_SetTransparency(true);                                                                          // enable transparency
  fontlib_SetWindow(TEXT_OFFSET, TEXT_OFFSET, GFX_LCD_WIDTH - TEXT_OFFSET, GFX_LCD_HEIGHT - TEXT_OFFSET); // define text window
  /* --------------- */

  /* local vars */
  uint8_t text_font_height = fontlib_GetCurrentFontHeight(),
          text_font_spacing = fontlib_GetSpaceBelow() + fontlib_GetSpaceAbove(); // font height

  score_xpos = TEXT_OFFSET + fontlib_GetStringWidth(SCORE_LABEL);
  score_bg = gfx_MallocSprite(0x7f, text_font_height);
  hydration_xpos = TEXT_OFFSET + ((HYDRATION_BAR_WIDTH - fontlib_GetStringWidth(HYDRATION_LABEL "100%")) / 2);
  hydration_ypos = GFX_LCD_HEIGHT - (TEXT_OFFSET + text_font_height + fontlib_GetSpaceBelow() + HYDRATION_BAR_HEIGHT);
  hydration_bg = gfx_MallocSprite(fontlib_GetStringWidth("100%"), text_font_height);

  clock_t clock_tick = clock(), // gametick limiter; last clock tick
      clock_now;                // gametick limiter; current clock tick
  /* --------- */

  wds_initGame();

  // begin drawing
  wds_beginDraw();

  // set global palette
  gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

  // begin gameplay loop
  do
  {
    // store key
    key = os_GetCSC();

    // properly hydrated
    if (hydration <= 101 * HYDRATION_FACTOR)
    { /* game functions */
      // hydration
      if (!hydration_cooldown)
      {
        // test to redraw background
        if (flags & 1)
        {
          // set flag
          flags &= ~1;
          // redraw
          wds_drawBg(0);
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
          wds_drawBg(1);
        }
      }

      // print score value
      gfx_Sprite(score_bg, score_xpos, TEXT_OFFSET);      // partial redraw
      fontlib_SetCursorPosition(score_xpos, TEXT_OFFSET); // position text cursor
      fontlib_DrawUInt(score / 10, 1);                    // pre-point value
      fontlib_DrawGlyph('.');                             // decimal point
      fontlib_DrawUInt(score % 10, 1);                    // post-point value

      // print on hydration update
      if (
          hydration_real * HYDRATION_FACTOR == hydration - (HYDRATION_FACTOR - 1) // is real proportional to raw? (with respect for grace period)
          || flags & 2)
      {
        // text value
        gfx_Sprite(hydration_bg, hydration_xpos, hydration_ypos);  // partial redraw
        fontlib_SetCursorPosition(hydration_xpos, hydration_ypos); // position text cursor (not centered; too lazy to figure out how to do that efficiently)
        fontlib_DrawUInt(hydration_real, 1);                       // value
        fontlib_DrawGlyph('%');                                    // percent sign
      }

      // resolve redraw
      if (flags & 2)
      {
        // game value labels
        fontlib_SetCursorPosition(TEXT_OFFSET, TEXT_OFFSET);
        fontlib_DrawString(SCORE_LABEL);
        fontlib_SetCursorPosition(TEXT_OFFSET + text_font_height, hydration_ypos);
        fontlib_DrawString(HYDRATION_LABEL);

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
      if (!(flags & 8))
      {
        // set hydration flag for game over state
        if (hydration & ~((unsigned)~0 >> 1))
          flags |= 4;
        else
          flags &= ~4;

        // draw game over background
        wds_drawBg(2 + !(flags & 4));

        // set text color
        fontlib_SetForegroundColor(0);

        // select game over message
        if (flags & 4) // dehydrate
        {
          wds_DrawScreenCenteredStringAbove("you perished of", 2);
          wds_DrawScreenCenteredStringAbove("  dehydration...", 1);
        }
        else // overhydrate
        {
          wds_DrawScreenCenteredStringAbove("you overhydrated", 2);
          wds_DrawScreenCenteredStringAbove("and drowned!", 1);
        }

        // finish game over output
        fontlib_SetFont(text_font_bold, 0);
        wds_DrawScreenCenteredStringBelow("game over", 0);
        fontlib_SetFont(text_font_regular, 0);

        // render
        wds_drawFrame();

        // update flags
        flags |= 8;
      }

      // await keypress
      wds_awaitCSC();

      // reinit on [2nd]/[enter]
      switch (key)
      {
      case sk_Enter:
      case sk_2nd:
        wds_initGame();
      }
    }
  }
  // loop until exit keypress
  while (key != sk_Clear);

  // end drawing
  wds_endDraw();

  // close data
  wds_closeData(data);

  return 0;
}

void wds_drawBg(uint8_t state)
{
  if (!(state & 2))
  {
    // decompress appropriate background asset
    zx7_Decompress(gfx_vbuffer, (state & 1) ? waterd33_compressed : waterd2_compressed);

    // update partial redraw buffers
    gfx_GetSprite(score_bg, score_xpos, TEXT_OFFSET);
    gfx_GetSprite(hydration_bg, hydration_xpos, hydration_ypos);

    // set redraw flag
    flags |= 2;
  }
  else
    // fill background with appropriate color
    gfx_FillScreen(5 + (state & 1));
}

void wds_initGame()
{
  // init from scratch
  hydration = (unsigned)(50.5 * HYDRATION_FACTOR);
  score = 0;
  hydration_cooldown = 0;
  saveTimer = 0;

  // (re)set text color
  fontlib_SetForegroundColor(1);

  // update flags
  flags = 1;
}
