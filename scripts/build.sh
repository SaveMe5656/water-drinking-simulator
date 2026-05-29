export PATH=~/CEdev/bin:$PATH

cd ..
make clean
make gfx; make

cd src/fonts
make
rm drsans-07-mod.bin; mv wdsFont.8xv ../../bin
