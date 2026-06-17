__water drinking simulator for the TI-84 Plus CE__  
official port by CommandCreep [[@SaveMe5656](https://github.com/SaveMe5656)]  
art by & inspiration thanks to *@motdab*

## changelog
### demo #5 patch #1
- adjust background drawing for palette changes

### demo #5
updated hydration bar with sprite-based rounded edges. also implemented decoding/encoding data after loading it from the TI AppVar
- updated hydration bar rendering
  + [ ] ~~standard rectangle-based rendering~~
  + [X] sprite-based rendering
- add mock hydration value display
- update AppVar saving/loading system to archive data AppVar
- add AppVar data decoding/encoding system

### demo #4
save data is working! took a lot of learning and headaches but saving and loading from a TI AppVar is now implemented  
still need to program something to return saved values after decoding them from their native ASCII-based format
- optimize drawing hydration meter
- add AppVar saving/loading system
- add data display

### demo #3
shape test. just rectangles since as far as I know those are the most I'll need for when I start work on the actual game as opposed to these demos
- rework how drawing frames is handled
- change keys for background switcher
- add mock hydration bar

### demo #2
fonts! text is definitely something that's needed, and this is a test to make sure printing text works correctly  
still just a tech demo; printing test characters is now a feature
- create a font
  + copy [Dr. Sans](https://github.com/drdnar/ce-fonts/tree/master/drsans) as a base
  + modify to be a bit goofier and fun-looking
  + setup tools for building
    * copy [`convhex.exe`](https://github.com/RoccoLoxPrograms/OSFonts/blob/main/fonts/OSLFONT/convhex.exe) from another font repository since I'm too #lazy to figure out how to obtain it normally
    * setup Linux bridge thru wine to convhex.exe (we LIKE Linux &#x1F601;)
    * setup font makefile
    * modify build script
- add font AppVar fetcher
- add glyph displayer


### demo #1
currently nothing but a tech demo; can switch between four background states, nothing else yet
- setup program properties
- setup some build scripts
- add background images
- add palette
- setup images & palette for convert
- add background switcher
