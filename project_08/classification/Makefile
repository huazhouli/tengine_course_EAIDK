export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig:$PKG_CONFIG_PATH

LDDFLAGS= -lpthread -ldl -lm -lrockchip_rga -lrockchip_mpp  `pkg-config --libs bladecv` `pkg-config --libs gtk+-3.0` `pkg-config --libs tengine`
INCFLAGS= `pkg-config --cflags bladecv` `pkg-config --cflags gtk+-3.0` `pkg-config --cflags tengine`
all:
	g++ -c common.cpp classification.cpp ${INCFLAGS}
	g++ common.o classification.o ${LDDFLAGS} -o classification
