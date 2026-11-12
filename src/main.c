/* libraries */
// standard libraries
#include <stdlib.h> // `gfx_MallocSprite`
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
#define progressbar_width 102
#define progressbar_height 7
#define progressbar_tile_width 1
#define progressbar_tile_height progressbar_height

#define TICK_RATE 10 // amount of ticks/second

#define HYDRATION_START_PERCENT 50                                           // hydration percent to init new game with
#define HYDRATION_FACTOR 15                                                  // factor between real hydration to get raw hydration value (real == raw / 15`)
#define HYDRATION_INCREMENT_PERCENT 30                                       // percent to increment hydration by
#define HYDRATION_INCREMENT (HYDRATION_INCREMENT_PERCENT * HYDRATION_FACTOR) // value to increment hydration by
#define HYDRATION_WARN_PERCENT 15                                            // percent to warn about low hydration (turn hydration bar red)
#define HYDRATION_COOLDOWN_START 23                                          // cooldown duration in ticks (10 ticks/sec * 2.28 seconds ~= 23 tick cooldown period)

#define UI_OFFSET 5                                                            // offset from screen edges to draw text at
#define HYDRATION_BAR_YPOS (GFX_LCD_HEIGHT - (UI_OFFSET + progressbar_height)) // y-position to draw the hydration bar

#define SCORE_LABEL "score"                               // label prefixing score value
#define HYDRATION_LABEL "hydration"                       // label prefixing hydration value
#define LABEL_SUFFIX ": "                                 // suffix after gui labels
#define SCORE_LABEL_FULL SCORE_LABEL LABEL_SUFFIX         // full gui label prefixing score value
#define HYDRATION_LABEL_FULL HYDRATION_LABEL LABEL_SUFFIX // full gui label prefixing hydration value

#define hydration_real (hydration / HYDRATION_FACTOR) // real hydration

#define text_font_regular fontlib_GetFontByIndex(text_font_ptr, 0) // regular font
#define text_font_bold fontlib_GetFontByIndex(text_font_ptr, 1)    // bold font

// pseudo-function for awaiting keypress via `os_GetCSC()`
#define wds_awaitCSC() \
  do                   \
  {                    \
    key = os_GetCSC(); \
  } while (!key)

// pseudo-function to get specific hydration bar sprite from hydration value and draw position
#define wds_getHydrationBarSprite(hyd, pos) progressbar_tiles[(pos) + (((((hyd) - (pos)) < progressbar_width) * (2 - !(flags & 16))) * progressbar_width)]

// pseudo-function to assist getting a y value relative to text line offset
#define wds_GetYLineOffset(lineOffset) (((lineOffset * 2) + 1) * text_font_height) + (((lineOffset) * 2) * text_font_spacing)

// pseudo-function for drawing centered text relative to the whole screen, with offset above the y-axis center line
#define wds_DrawCenteredStringAbove(string, lineOffset)                                   \
  fontlib_SetCursorPosition(                                                              \
      (GFX_LCD_WIDTH - fontlib_GetStringWidth(string)) / 2,                               \
      (GFX_LCD_HEIGHT - (wds_GetYLineOffset(lineOffset) + fontlib_GetSpaceBelow())) / 2); \
  fontlib_DrawString(string)

// pseudo-function for drawing centered text relative to the whole screen, with offset below the y-axis center line
#define wds_DrawCenteredStringBelow(string, lineOffset)                                   \
  fontlib_SetCursorPosition(                                                              \
      (GFX_LCD_WIDTH - fontlib_GetStringWidth(string)) / 2,                               \
      (GFX_LCD_HEIGHT + (wds_GetYLineOffset(lineOffset) + fontlib_GetSpaceAbove())) / 2); \
  fontlib_DrawString(string)

// pseudo-function for drawing an unsigned integer value centered roughly relative to the whole screen, with offset below the y-axis center line
#define wds_DrawCenteredLabeledUIntBelow(label, value, lineOffset)                        \
  fontlib_SetCursorPosition(                                                              \
      (GFX_LCD_WIDTH / 2) - fontlib_GetStringWidth(label),                                \
      (GFX_LCD_HEIGHT + (wds_GetYLineOffset(lineOffset) + fontlib_GetSpaceAbove())) / 2); \
  fontlib_DrawString(label LABEL_SUFFIX);                                                 \
  fontlib_DrawUInt(value, 1)
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
| 3   | `4`   | game over type  |
| 4   | `8`   | game over draw  |
| 5   | `16`  | hydration state |
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
  char *text_font_ptr = wds_getFont("Dr. Aller");
  // return if font init fail
  if (text_font_ptr == NULL)
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
  fontlib_SetFont(text_font_regular, 0);                                                          // use regular font
  fontlib_SetTransparency(true);                                                                  // enable transparency
  fontlib_SetWindow(UI_OFFSET, UI_OFFSET, GFX_LCD_WIDTH - UI_OFFSET, GFX_LCD_HEIGHT - UI_OFFSET); // define text window
  /* --------------- */

  /* local vars */
  uint8_t text_font_height = fontlib_GetCurrentFontHeight(),
          text_font_spacing = fontlib_GetSpaceBelow() + fontlib_GetSpaceAbove(); // font height

  score_xpos = UI_OFFSET + fontlib_GetStringWidth(SCORE_LABEL_FULL);
  score_bg = gfx_MallocSprite(0x7f, text_font_height);
  uint8_t hydration_xpos_label = UI_OFFSET + ((progressbar_width - fontlib_GetStringWidth(HYDRATION_LABEL_FULL "100%")) / 2); // x-position of hydration value label
  hydration_xpos = hydration_xpos_label + fontlib_GetStringWidth(HYDRATION_LABEL_FULL);
  hydration_ypos = HYDRATION_BAR_YPOS - (text_font_height + fontlib_GetSpaceBelow());
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
    {
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
          wds_drawBg(0);
        }

        // hydrate on keypress
        switch (key)
        {
        case sk_Enter:
        case sk_2nd:
          // update hydration value
          hydration += HYDRATION_INCREMENT;
          // init hydration cooldown
          hydration_cooldown = HYDRATION_COOLDOWN_START;
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
      gfx_Sprite_NoClip(score_bg, score_xpos, UI_OFFSET); // partial redraw
      fontlib_SetCursorPosition(score_xpos, UI_OFFSET);   // position text cursor
      fontlib_DrawUInt(score / 10, 1);                    // pre-point value
      fontlib_DrawGlyph('.');                             // decimal point
      fontlib_DrawUInt(score % 10, 1);                    // post-point value

      // draw on hydration update
      if (
          hydration_real * HYDRATION_FACTOR == hydration - (HYDRATION_FACTOR - 1) // is real proportional to raw? (with respect for grace period)
          || flags & 2)
      {
        // print text value
        gfx_Sprite_NoClip(hydration_bg, hydration_xpos, hydration_ypos); // partial redraw
        fontlib_SetCursorPosition(hydration_xpos, hydration_ypos);       // position text cursor (not centered; too lazy to figure out how to do that efficiently)
        fontlib_DrawUInt(hydration_real, 1);                             // value
        fontlib_DrawGlyph('%');                                          // percent sign

        // change progress bar color on hydration/flag mismatch
        if ((hydration_real <= HYDRATION_WARN_PERCENT) == !(flags & 16))
        {
          // update flag
          if (hydration_real > HYDRATION_WARN_PERCENT)
            flags &= ~16;
          else
            flags |= 16;
          // update progress bar color
          for (uint8_t i = 0; i <= hydration_real; i++)
            gfx_Sprite_NoClip(
                wds_getHydrationBarSprite(hydration_real, i),
                UI_OFFSET + i,
                HYDRATION_BAR_YPOS);
        }

        // empty last hydration bar segment
        gfx_Sprite_NoClip(progressbar_tiles[hydration_real + 1], UI_OFFSET + hydration_real + 1, HYDRATION_BAR_YPOS);
      }

      // resolve redraw
      if (flags & 2)
      {
        // game value labels
        fontlib_SetCursorPosition(UI_OFFSET, UI_OFFSET);
        fontlib_DrawString(SCORE_LABEL_FULL);
        fontlib_SetCursorPosition(hydration_xpos_label, hydration_ypos);
        fontlib_DrawString(HYDRATION_LABEL_FULL);

        // hydration bar
        for (uint8_t i = 0; i < progressbar_width; i++)
          gfx_Sprite_NoClip(
              wds_getHydrationBarSprite(hydration_real, i),
              UI_OFFSET + i,
              HYDRATION_BAR_YPOS);

        // update flags
        flags &= ~2;
      }

      // increment/decrement
      hydration--;
      score++;

      // print screen
      wds_drawFrame();
      /* -------------- */

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
        // set hydration flag for game over type
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
          wds_DrawCenteredStringAbove("you perished of", 2);
          wds_DrawCenteredStringAbove("  dehydration...", 1);
        }
        else // overhydrate
        {
          wds_DrawCenteredStringAbove("you overhydrated", 2);
          wds_DrawCenteredStringAbove("and drowned!", 1);
        }

        // finish game over output
        fontlib_SetFont(text_font_bold, 0);                           // bold font
        wds_DrawCenteredStringBelow("game over", 0);                  // "game over"
        fontlib_SetFont(text_font_regular, 0);                        // regular font
        wds_DrawCenteredLabeledUIntBelow(SCORE_LABEL, score / 10, 1); // pre-point score
        fontlib_DrawGlyph('.');                                       // decimal point
        fontlib_DrawUInt(score % 10, 1);                              // post-point score

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
    gfx_GetSprite(score_bg, score_xpos, UI_OFFSET);
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
  hydration = (unsigned)((((2 * (HYDRATION_START_PERCENT)) + 1) * HYDRATION_FACTOR) / 2);
  score = 0;
  hydration_cooldown = 0;
  saveTimer = 0;

  // (re)set text color
  fontlib_SetForegroundColor(1);

  // update flags
  flags = 1;
}
