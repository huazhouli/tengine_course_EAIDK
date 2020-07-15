export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig:$PKG_CONFIG_PATH

LDDFLAGS= -lrockchip_rga -lrockchip_mpp -ldrm -lcam_ia -lpthread  `pkg-config --libs bladecv` `pkg-config --libs tengine` `pkg-config --libs gtk+-3.0` 
INCFLAGS= `pkg-config --cflags bladecv` `pkg-config --cflags tengine` `pkg-config --cflags gtk+-3.0`
all:
	g++ -c usb_cam.cpp usb-face.cpp mtcnn.cpp mtcnn_utils.cpp ${INCFLAGS}
	g++ mtcnn.o mtcnn_utils.o usb_cam.o usb-face.o ${LDDFLAGS} -o usb-face
