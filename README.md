> [!WARNING]
> this version is still under heavy development; expect missing or broken features

# water drinking simulator for the TI-84 Plus CE
a fairly simple calculator game about staying properly hydrated

official port of Web game [water drinking simulator](https://saveme5656.github.io/water-drinking-simulator) to the TI-84 Plus CE calculator


## details
drink water to avoid dehydrating; stop before you overhydrate (drown)

additional details are listed with the Web version:  
https://saveme5656.github.io/water-drinking-simulator/about#gameplay

releases are listed on [wds's release section](https://github.com/SaveMe5656/water-drinking-simulator/releases), prefixed as wdsCE followed by the version number


## compiling
make sure you have GNU `make`, the [CE C Toolchain](https://github.com/SaveMe5656/water-drinking-simulator/releases), and download or clone wdsCE's source code repository

at the root of the repository, run `make gfx` and `make` to build the program. it'll be spit out in the `bin` directory with the extension `.8xp`

in `src/fonts`, run `make` to build the font. the `.8xv` AppVar will appear in the same folder

```
cd wdsCE/
make gfx && make
cd src/fonts/
make
```


## extra credits
thanks again *@motdab* for Gerald and his design, as well as inspiration for the game as a whole

additional thanks go to the [CE Programming](https://github.com/CE-Programming) group for their [toolchain and documentation](https://ce-programming.github.io/toolchain), both essential tools to this project

thank you [@drdnar](https://github.com/drdnar) for the [Dr. Sans](https://github.com/drdnar/ce-fonts/tree/master/drsans) font that was used as a base for this program's font
