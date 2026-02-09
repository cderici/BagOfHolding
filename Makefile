CXX      := clang++-20
CXXFLAGS := -std=c++23 -D__cpp_lib_source_location=0 -O2 -Wall -Wextra -Wpedantic
LDFLAGS  := -stdlib=libstdc++
LDLIBS   := -lpqxx -lpq

BUILDDIR := build

BACKEND_NAME := backend
BACKEND_SRC  := backend/state.cpp
BACKEND_BIN  := $(BUILDDIR)/$(BACKEND_NAME)

.PHONY: all backend run-backend clean

all: backend

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

backend: $(BACKEND_BIN)

$(BACKEND_BIN): $(BACKEND_SRC) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(BACKEND_SRC) $(LDLIBS) -o $(BACKEND_BIN)

run-backend: backend
	./$(BACKEND_BIN)

clean:
	rm -rf $(BUILDDIR)
