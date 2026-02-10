CXX      := clang++-20
CXXFLAGS := -std=c++23 -D__cpp_lib_source_location=0 -O2 -Wall -Wextra -Wpedantic
LDFLAGS  := -stdlib=libstdc++
LDLIBS   := -lpqxx -lpq

BUILDDIR := build

BACKEND_NAME := backend
BACKEND_SRC  := backend/main.cpp backend/state.cpp
BACKEND_BIN  := $(BUILDDIR)/$(BACKEND_NAME)

STATE_TEST_SOURCES := $(wildcard tests/backend/state/*_test.cpp)
STATE_TEST_BINS    := $(patsubst tests/backend/state/%_test.cpp,$(BUILDDIR)/%_test,$(STATE_TEST_SOURCES))

.PHONY: all backend run-backend test test-bin clean

all: backend

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

backend: $(BACKEND_BIN)

$(BACKEND_BIN): $(BACKEND_SRC) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(BACKEND_SRC) $(LDLIBS) -o $(BACKEND_BIN)

test-bin: $(STATE_TEST_BINS)

$(BUILDDIR)/%_test: tests/backend/state/%_test.cpp backend/state.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< backend/state.cpp $(LDLIBS) -o $@

test: test-bin
	@status=0; for test_bin in $(STATE_TEST_BINS); do ./$$test_bin || status=1; done; exit $$status

run-backend: backend
	./$(BACKEND_BIN)

clean:
	rm -rf $(BUILDDIR)
