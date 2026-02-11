CXX      := clang++-20
CXXFLAGS := -std=c++23 -D__cpp_lib_source_location=0 -O2 -Wall -Wextra -Wpedantic
LDFLAGS  := -stdlib=libstdc++
LDLIBS   := -lpqxx -lpq

BUILDDIR := build

BACKEND_NAME := backend
STATE_SRC    := $(wildcard backend/state/*.cpp)
BACKEND_SRC  := backend/main.cpp $(STATE_SRC)
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

$(BUILDDIR)/%_test: tests/backend/state/%_test.cpp $(STATE_SRC) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< $(STATE_SRC) $(LDLIBS) -o $@

test: test-bin
	@state_status=0; for test_bin in $(STATE_TEST_BINS); do ./$$test_bin || state_status=1; done; if [ $$state_status -eq 0 ]; then printf "\nState Layer Tests : PASS\n\n"; else printf "\nState Layer Tests : FAIL\n\n"; fi; backend_status=$$state_status; if [ $$backend_status -eq 0 ]; then printf -- "-- BACKEND TESTS : PASS\n\n"; else printf -- "-- BACKEND TESTS : FAIL\n\n"; fi; exit $$backend_status

run-backend: backend
	./$(BACKEND_BIN)

clean:
	rm -rf $(BUILDDIR)
