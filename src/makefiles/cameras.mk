# ░█▀▀░█▀█░█▄█░█▀▀░█▀▄░█▀█
# ░█░░░█▀█░█░█░█▀▀░█▀▄░█▀█
# ░▀▀▀░▀░▀░▀░▀░▀▀▀░▀░▀░▀░▀

cameras:	$(BUILD_DIR) $(BUILD_DIR)/veho $(BUILD_DIR)/eakins $(BUILD_DIR)/webcam $(BUILD_DIR)/canny

# TODO: Find out why we cannot use 'zig c++' here (fails to link)
$(BUILD_DIR)/veho:	$(SRC_DIR)/video/capture.cpp
	g++ -DCAPTURE_DEVICE=\"/dev/v4l/by-id/usb-Vimicro_Co._ltd_Vimicro_USB2.0_UVC_PC_Camera-video-index0\" -DCAPTURE_X_RESOLUTION=640 -DCAPTURE_Y_RESOLUTION=480 -DCAMERA_NAME=\"veho\" -DUSERNAME=\"$(USERNAME)\" -std=c++11 $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

$(BUILD_DIR)/eakins:	 $(SRC_DIR)/video/capture.cpp
	g++ -DCAPTURE_DEVICE=\"/dev/v4l/by-id/usb-VXIS_Inc_ezcap_U3_capture-video-index0\" -DCAPTURE_X_RESOLUTION=1920 -DCAPTURE_Y_RESOLUTION=1080 -DCAMERA_NAME=\"eakins\" -DUSERNAME=\"$(USERNAME)\" -std=c++11 $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

$(BUILD_DIR)/webcam:	$(SRC_DIR)/video/capture.cpp
	g++ -DCAPTURE_DEVICE=\"/dev/v4l/by-id/usb-GENERAL_GENERAL_WEBCAM_JH0319_20210727_v017-video-index0\" -DCAPTURE_X_RESOLUTION=1920 -DCAPTURE_Y_RESOLUTION=1080 -DCAMERA_NAME=\"webcam\" -DCAPTURE_USE_MJPEG -DUSERNAME=\"$(USERNAME)\" -std=c++11 $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

$(BUILD_DIR)/ueye_opencv.o:	$(SRC_DIR)/video/ueye_opencv.cpp
	g++ -c -I $(LIBBLASTPIT_DIR) -I $(REDIST_DIR) $(shell pkg-config opencv4 --cflags) -o $@ $^

$(BUILD_DIR)/ueye:	$(LIBBLASTPIT_OBJS) $(SRC_DIR)/video/ueye.c $(SRC_DIR)/video/ueye_opencv.h $(BUILD_DIR)/ueye_opencv.o
	g++ -I $(LIBBLASTPIT_DIR) -I $(REDIST_DIR) $(shell pkg-config opencv4 --cflags) -o $@ $(SRC_DIR)/video/ueye.c -L $(REDIST_DIR) $(BUILD_DIR)/ueye_opencv.o -lueye_api -pthread $(shell pkg-config opencv4 --libs) $(LIBBLASTPIT_OBJS)

# Canny edge detector demo
$(BUILD_DIR)/canny:	$(SRC_DIR)/video/canny.cpp
	g++ $(CXXFLAGS) -o $@ $(shell pkg-config opencv4 --cflags) $^ $(shell pkg-config opencv4 --libs)

ueye:	$(BUILD_DIR) $(BUILD_DIR)/ueye
