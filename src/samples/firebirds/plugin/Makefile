PLUGIN_NAME = collisiondetector

INCLUDE = -I.
INCLUDE += -I$(AVG_PATH)/libavg/src
CONFIG = $(shell pkg-config libxml-2.0 --cflags)
CONFIG += $(shell python-config --includes)

CXXOPT = -Wall -Wno-invalid-offsetof -O3 -gstabs -fPIC
CXX = g++ $(CXXOPT) $(INCLUDE) $(CONFIG)
ifneq (,$(findstring Darwin,$(shell uname)))
LINKOPT = -bundle -flat_namespace -undefined suppress
else
ifneq (,$(findstring Linux,$(shell uname)))
LINKOPT = -fPIC -shared -shared-libgcc $(AVG_DEPLOY_PATH)/avg.so
else
	@echo "Unknown platform"
	@exit 1
endif
endif

all: testpath $(PLUGIN_NAME).so

testpath:
ifndef AVG_PATH
	@echo "Please define AVG_PATH"
	@exit 1
endif

$(PLUGIN_NAME).so: CollisionDetector.o
	g++ $(LINKOPT) -o $@ $^

CollisionDetector.o: CollisionDetector.cpp

deploypath:
AVG_DEPLOY_PATH = $(shell python -c "import os;import libavg;print os.path.dirname(libavg.__file__)" 2>/dev/null)

clean:
	rm -f *.o *~ $(PLUGIN_NAME).so

