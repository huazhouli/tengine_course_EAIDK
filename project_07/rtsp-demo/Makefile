export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig:$PKG_CONFIG_PATH

LDDFLAGS= -lrtspclient -lrockchip_rga -lrockchip_mpp -lcam_engine_cifisp -lcam_ia -lpthread  `pkg-config --libs bladecv` `pkg-config --libs gtk+-3.0` 
INCFLAGS= `pkg-config --cflags bladecv` `pkg-config --cflags gtk+-3.0`
all:
	g++ -c ipc_rtsp.cpp rtsp-demo.cpp ${INCFLAGS}
	g++ ipc_rtsp.o rtsp-demo.o ${LDDFLAGS} -o rtsp-demo
