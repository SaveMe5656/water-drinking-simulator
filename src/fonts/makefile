all: DrAller.8xv

draller-07.bin: draller-07-normal.convfont draller-07-bold.convfont
	convfont -o fontpack -P "Calculator 1252" \
	-N "Dr. Aller" \
	-D "An aesthetically playful font, forked off of Dr. Sans by DrDnar and based on Aller by Dalton Maag Ltd." \
	-A "CommandCreep" \
	-V "5/25/2026" \
	-t draller-07-normal.convfont \
	-t draller-07-bold.convfont \
	draller-07.bin

DrAller.8xv: draller-07.bin
	./convhex -a -v -n DrAller draller-07.bin DrAller.8xv
