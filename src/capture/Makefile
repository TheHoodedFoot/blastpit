SOURCES:=capture.cpp

CXX      = ccache g++
CXXFLAGS = -Wall -ggdb3 -O0

all: veho eakins

veho: $(SOURCES)
	$(CXX) -DCAPTURE_DEVICE=2 -DCAPTURE_X_RESOLUTION=640 -DCAPTURE_Y_RESOLUTION=480 -DCAMERA_NAME=\"$@\" -std=c++11 $(CXXFLAGS) -o $@ $^ $(shell pkg-config opencv --libs)

eakins: $(SOURCES)
	$(CXX) -DCAPTURE_DEVICE=0 -DCAPTURE_X_RESOLUTION=1920 -DCAPTURE_Y_RESOLUTION=1080 -DCAMERA_NAME=\"$@\" -std=c++11 $(CXXFLAGS) -o $@ $^ $(shell pkg-config opencv --libs)

clean:
	rm -f $(OUTPUT) *.png *.jp?g
