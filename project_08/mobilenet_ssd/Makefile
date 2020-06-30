export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig:$PKG_CONFIG_PATH

LDDFLAGS= -lpthread -ldl -lm -lrockchip_rga -lrockchip_mpp -lcam_engine_cifisp -lcam_ia  `pkg-config --libs bladecv` `pkg-config --libs gtk+-3.0` `pkg-config --libs tengine`
INCFLAGS= `pkg-config --cflags bladecv` `pkg-config --cflags gtk+-3.0` `pkg-config --cflags tengine`
all:
	g++ -c common.cpp mipi_cam.cpp mobilenet_ssd.cpp ${INCFLAGS}
	g++ common.o mipi_cam.o mobilenet_ssd.o ${LDDFLAGS} -o mobilenet_ssd
