I am sincerely sorry

water drinking simulator by CommandCreep <ins>[@saveme5656]</ins> <br>
art by and inspiration thanks to <ins>@motdab</ins>

current version: 2.1



## Changelog:
### ver. Alpha
  * program general structure
  * add reset button
  * add assets
  * implement drinking
    - probably a bad idea to do it based on audio playback time in hindsight but whatever ![:nothoughts:](https://cdn.discordapp.com/emojis/1201246313493311488.webp?size=16)

### ver. Beta
  * add inputs for difficulty parameters
  * implement gameplay
    - hydration starts at 50%
    - Bottle Capacity = % to be added to hydration level
    - Dehydration Time: secs to dehydrate

### ver. 1.0
  * make standard function conditional to proper hydration
  * add dehydration screen
  * add overhydration screen

### ver. 1.1
  * add 1.0 changelog (yes I somehow forgot it **:p**)
  * tweak colors
  
### ver. 1.2
  * tweak default bottle capacity
  
### ver. 1.3
  * add variable for round absolute hydration
  * tweak hydration level to be based on round absolute hydration
  
### ver. 2.0
  * add time-based scoring system
  
### ver. 2.1
  * tweak hydration progressbar warning color level

### ver. 3.0
  * attempt adding BGM
  * tweak hydration bar to have rounded ends
  * fix hydration bar display
  * update library p5.js
  * create cookie library
    - needed to use cookies for saving things in-game and a library seemed like a good choice since I could use it in more places than just here
  * fix BGM ![:sneakysnitch:](https://cdn.discordapp.com/emojis/1388073015929212929.webp?size=16)
  * make highscore saving preparations (stop game from looping on game over)
  * remove random line of stray commented-out code
  * make music pause on game over
  * implement highscore saving via browser cookies
  * implement autosave via browser cookies
  * add options section on HTML page above difficulty parameters
  * add autosave toggling

### ver. 3.1
  * fix loading from cookies
    - the game was loading from cookies even when empty; resulted in values being `NaN` and crashing the game
