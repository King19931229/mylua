# https://stackoverflow.com/questions/2908057/can-i-compile-all-cpp-files-in-src-to-os-in-obj-then-link-to-binary-in
INC_DIR = .
SRC_DIR = .
OBJ_DIR = ./intermediates
# All source files listed automatically
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
# You also need a list of the object files (one .o per .cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
DEFS = -D _DEBUG -D UNICODE -D _UNICODE
CXX = g++
# https://nathandumont.com/blog/automatically-detect-changes-in-header-files-in-a
CXX_FLAGS = -std=c++11 -ggdb -Wall -O0 $(DEFS) -I$(INC_DIR) -MD

main.exe: $(OBJ_FILES)
	$(CXX) $(LD_FLAGS) -o $@ $^
	@echo "target: " $@
	@echo "source: " $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p intermediates
	$(CXX) $(CXX_FLAGS) -c -o $@ $<
	@echo "target: " $@
	@echo "source: " $<

clean:
	rm -f $(OBJ_FILES) main.exe
	rm -rf intermediates

-include $(OBJ_FILES:.o=.d)