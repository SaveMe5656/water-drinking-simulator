all: MrSans.8xv

mrsans-07.bin: mrsans-07-normal.convfont mrsans-07-bold.convfont
	convfont -o fontpack -P "Calculator 1252" \
	-N "Mr. Sans" \
	-D "An aesthetically fun modification of drdnar's Dr. Sans" \
	-A "CommandCreep" \
	-V "5/25/2026" \
	-t mrsans-07-normal.convfont \
	-t mrsans-07-bold.convfont \
	mrsans-07.bin

MrSans.8xv: mrsans-07.bin
	./convhex -a -v -n wdsFont mrsans-07.bin MrSans.8xv
