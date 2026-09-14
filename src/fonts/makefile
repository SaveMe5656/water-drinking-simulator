all: MrSans.8xv

drsans-07-mod.bin: drsans-07-normal-mod.convfont drsans-07-bold-mod.convfont
	convfont -o fontpack -P "Calculator 1252" \
	-N "Mr. Sans" \
	-D "An aesthetically fun modification of drdnar's Dr. Sans" \
	-A "CommandCreep" \
	-V "5/25/2026" \
	-t drsans-07-normal-mod.convfont \
	-t drsans-07-bold-mod.convfont \
	drsans-07-mod.bin

MrSans.8xv: drsans-07-mod.bin
	./convhex -a -v -n wdsFont drsans-07-mod.bin MrSans.8xv
