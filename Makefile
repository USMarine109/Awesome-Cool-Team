CXX ?= g++
CXXFlags ?= -std=c++26 -o2 -Wall -Wextra -pedantic
INC := -Iinclude
SB_Root ?= $(HOME)/sb_open_data
MATCH ?= 38696685

APP_SRC := src/main.cpp src/Load_Dataset.cpp src/Fenwick_Tree.cpp src/Segment_Tree.cpp src/Load_Dataset.cpp
HDRS	:= $(wildcard include/*.h)

all: momemtum_tracker test_trees

monmentum_tracker: $(APP_SRC) $(HDRS) $(CXX) $(CXXFLAGS) $(INC) $(APP_SRC) -o $@

test_trees: tests/test_trees.cpp src/Load_Dataset.cpp.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) $(INC) tests/test_trees.cpp src/Momentum.cpp -o $@

test: test_trees
	./test_trees

data: data/events.csv
data/events.csv:
	scripts/fetch_data.sh $(SB_ROOT) 43 106 3
	python3 scripts/preprocess.py --sb-root $(SB_ROOT) --competition 43 --seasons 106 3 --out data

demo: momentum_tracker data/events.csv
	./momentum_tracker --match $(MATCH) --range 30 45 --shifts --bench --plot

clean:
	rm -f momentum_tracker test_trees

.PHONY: all test data demo clean