export PATH=~/CEdev/bin:$PATH

cd ..
make gfx; make

cd src/fonts
make
rm mrsans-07.bin; mv MrSans.8xv ../../bin
