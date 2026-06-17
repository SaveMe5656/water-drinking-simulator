#include <stdio.h>
// #include <stdlib.h> // unknown if needed
#include <stdint.h> // included by <graphx.h>, <fontlibc.h>, <ti/getcsc.h>, "data.h"
// #include <stdbool.h> // included by "data.h"; unknown if needed

#include <graphx.h>
#include "gfx/gfx.h"

#include <fontlibc.h>
#include "fonts/calc1252.h"

#include <compression.h>
#include <ti/getcsc.h>

#include "data.h"
#include "draw.h"
#include "text.h"

/*

draw imgname as background
```c
zx7_Decompress(gfx_vram, imgname_compressed);
```

fill screen with color from palette
```c
gfx_FillScreen(0);
```

check if two arrays are identical
```c
#include <string.h>
int foo[3] = {1, 2, 3};
int bar[3] = {1, 2, 3};
uint8_t foo_isIdenticalTo_bar = (sizeof(foo) == sizeof(bar) && memcmp(foo, bar, sizeof(foo)) == 0);
```

IBM437 character table
```txt
   0 1 2 3 4 5 6 7 8 9 A B C D E F

0    ☺ ☻ ♥ ♦ ♣ ♠ • ◘ ○ ◙ ♂ ♀ ♪ ♬ ☼
1  ▶ ◀ ↕ ‼ ¶ § ▬ ↨ ↑ ↓ → ← ⌙ ↔ ▲ ▼
2    ! " # $ % & ' ( ) * + , - . /
3  0 1 2 3 4 5 6 7 8 9 : ; < = > ?
4  @ A B C D E F G H I J K L M N O
5  P Q R S T U V W X Y Z [ \ ] ^ _
6  ` a b c d e f g h i j k l m n o
7  p q r s t u v w x y z { | } ~ ⌂

8  Ç ü é â ä à å ç ê ë è ï î ì Ä Å
9  É æ Æ ô ö ò û ù ÿ Ö Ü ¢ £ ¥ ₧ ƒ
A  á í ó ú ñ Ñ ª º ¿ ⌐ ¬ ½ ¼ ¡ « »
B  ░ ▒ ▓ │ ┤ ╡ ╢ ╖ ╕ ╣ ║ ╗ ╝ ╜ ╛ ┐
C  └ ┴ ┬ ├ ─ ┼ ╞ ╟ ╚ ╔ ╩ ╦ ╠ ═ ╬ ╧
D  ╨ ╤ ╥ ╙ ╘ ╒ ╓ ╫ ╪ ┘ ┌ █ ▄ ▌ ▐ ▀
E  α β Γ π Σ σ μ γ Φ θ Ω δ ∞ ∅ ∈ ∩
F  ≡ ± ≥ ≤ ⌠ ⌡ ÷ ≈ ° ∙ ⋅ √ ⁿ ² ∎  
```

*/

int main()
{
  // init vars for demo
  uint8_t key;
  uint8_t bg_scene = 0, bg_flag = 1;
  uint8_t text_glyph = 0, text_style = 0, text_flag = 1;
  uint8_t shape_flag = 1, shape_width = 50, shape_colorOffset;
  uint8_t data_flag = 0;
  uint16_t data_hydration_raw = shape_width * 30;

  // open data
  char *data;
  wds_openData(&data);

  // get Mr. Sans font (modified Dr. Sans by DrDnar)
  char *text_font = wds_getFont("Mr. Sans");

  // set font
  fontlib_SetFont(fontlib_GetFontByIndex(text_font, text_style), 0);

  // set text drawing properties
  // fontlib_SetTransparency(true);
  fontlib_SetColors(0, 1);

  // begin drawing
  wds_beginDraw();

  // set global palette
  gfx_SetPalette(wds_palette, sizeof_wds_palette, 0);

  // begin loop
  do
  {
    // test to draw background if any draw flags enabled
    // note to self: don't redraw entire background in proper version
    if (bg_flag || text_flag || shape_flag || data_flag)
    {
      // test scene value
      switch (3 & bg_scene)
      {
        // idle if 0
      case 0:
        zx7_Decompress(gfx_vbuffer, waterd2_compressed);
        break;

        // drinking if 1
      case 1:
        zx7_Decompress(gfx_vbuffer, waterd33_compressed);
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

      text_flag = 1;
      shape_flag = 1;
    }

    // encode/decode save data, display
    if (data_flag)
    {
      wds_encode(&data_hydration_raw, 2, &data, 3);
    }

    // draw current glyph and debug text if flag enabled
    if (text_flag)
    {
      // font glyph
      fontlib_SetCursorPosition(0, 0);
      fontlib_DrawGlyph(text_glyph);
      fontlib_SetCursorPosition(0, 10);
      fontlib_DrawUInt(text_glyph, 1);

      // data properties
      fontlib_SetCursorPosition(0, 22);
      fontlib_DrawString(data);
      data_hydration_raw = shape_width * 30;
      fontlib_SetCursorPosition(0, 32);
      fontlib_DrawUInt(data_hydration_raw, 1);
      fontlib_DrawString(", ");
      fontlib_DrawUInt(wds_decodeValue(data, 3), 1);

      // hydration bar properties
      fontlib_SetCursorPosition(0, 45);
      fontlib_DrawUInt(shape_width, 1);
      fontlib_SetCursorPosition(0, 55);
      fontlib_DrawUInt(shape_colorOffset, 1);
    }

    // draw test shapes
    if (shape_flag)
    {
      // set progress bar color
      if (shape_width > 15)
        shape_colorOffset = 1;
      else
        shape_colorOffset = 2;

      // draw progress bar
      // note to future self: when properly drawing progress bar, draw only the segments needed to update it instead of all progress bar segments
      for (uint8_t i = 0; i < 102; i++)
      {
        // init shape modifier value
        uint8_t shape_mod = 0;

        // modify shape color
        if (i < shape_width + 1)
          shape_mod += (progressbar_num_tiles / 3) * shape_colorOffset;

        // modify shape end
        if (i < ((progressbar_num_tiles / 3) - 1))
          shape_mod += i + 1;
        else if (i > 102 - ((progressbar_num_tiles / 3) - 1))
          shape_mod += 102 - i;

        gfx_Sprite(progressbar_tiles[shape_mod], 4 + i, GFX_LCD_HEIGHT - 11);
      }
    }

    // draw frame if any draw flags enabled
    if (bg_flag || text_flag || shape_flag)
      wds_drawFrame();

    // store key
    key = os_GetCSC();

    // enable draw flags
    bg_flag = 1;
    text_flag = 1;
    shape_flag = 1;
    data_flag = 1;

    // test pressed key for scene switching
    switch (key)
    {
    // [up]: increment scene
    case sk_Up:
      bg_scene++;
      break;

    // [down]: decrement scene
    case sk_Down:
      bg_scene--;
      break;

    // default: disable bg draw flag
    default:
      bg_flag = 0;
      break;
    }

    // test pressed key for glyph printing
    switch (key)
    {
    // [+]: increment glyph code
    case sk_Add:
      text_glyph++;
      break;

    // [-]: decrement glyph code
    case sk_Sub:
      text_glyph--;
      break;

    // [0]-[9]: push digit to the end of current glyph's decimal value
    case sk_0:
      text_glyph *= 10;
      break;
    case sk_1:
    case sk_2:
    case sk_3:
    case sk_4:
    case sk_5:
    case sk_6:
    case sk_7:
    case sk_8:
    case sk_9:
      text_glyph *= 10;
      text_glyph += (key % 8 - 1) * 3 - key / 8 + 2;
      break;

    // [del]: pop last digit of current glyph's decimal value
    case sk_Del:
      text_glyph /= 10;
      break;

    // [*]: change font style
    case sk_Mul:
      text_style++;
      fontlib_SetFont(fontlib_GetFontByIndex(text_font, 1 & text_style), 0);
      break;

    // default: disable text draw flag
    default:
      text_flag = 0;
      break;
    }

    // test pressed key for shape drawing
    switch (key)
    {
    // [left]: decrease width
    case sk_Left:
      shape_width > 0 && shape_width--;
      break;

    // [right]: decrease width
    case sk_Right:
      shape_width < 100 && shape_width++;
      break;

    // default: disable shape draw flag
    default:
      shape_flag = 0;
      break;
    }

    // test pressed key for data updating
    switch (key)
    {
    case sk_Enter:
      data_flag = 1;
      break;

    default:
      data_flag = 0;
      break;
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
