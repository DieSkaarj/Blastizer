CXX = g++ --std=c++20 -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
CXXFLAGS = -Wall 
INCLUDES = -Iinclude -I/mingw64/include/ImageMagick-7 -I/mingw64/include 
MAGICK = `Magick++-config --cppflags --cxxflags --ldflags`
FLTK = `fltk-config -g --use-images --cxxflags --ldflags` 
LIBRARIES = 
SRC = src/*.cxx

bin/BlastEd.exe: $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(INCLUDES) -Wl,-Bdynamic $(MAGICK) $(FLTK) $(LIBRARIES)
