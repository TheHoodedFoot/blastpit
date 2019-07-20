SOURCES:=webcam_capture.cpp
OUTPUT=cap

CXX      = ccache g++
CXXFLAGS = -Wall -ggdb3 -O0

all: $(SOURCES) $(OUTPUT)
	rm -f webcam_output.jpeg
	./$(OUTPUT)
	feh webcam_output.jpeg

$(OUTPUT): $(SOURCES)
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ $^ $(shell pkg-config opencv --libs)

clean:
	rm -f $(OUTPUT) *.png *.jp?g
