all: wdsFont.8xv

drsans-07-mod.bin: drsans-07-normal-mod.convfont drsans-07-bold-mod.convfont
	convfont -o fontpack -P "Calculator 1252" \
	-N "Mr. Sans" \
	-D "A aesthetically fun mod of Dr. Sans, notably used in CommandCreep's water drinking simulator" \
	-A "CommandCreep" \
	-V "5/25/2026" \
	-t drsans-07-normal-mod.convfont \
	-t drsans-07-bold-mod.convfont \
	drsans-07-mod.bin

wdsFont.8xv: drsans-07-mod.bin
	./convhex -a -v -n wdsFont drsans-07-mod.bin wdsFont.8xv
