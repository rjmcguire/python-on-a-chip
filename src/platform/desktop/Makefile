# PyMite Configuration
PLATFORM := $(notdir $(CURDIR))
PM_LIB_ROOT = pmvm_$(PLATFORM)
PM_LIB_FN = lib$(PM_LIB_ROOT).a
PM_LIB_PATH = ../../vm/$(PM_LIB_FN)
PM_USR_SOURCES = main.py
PMIMGCREATOR := ../../tools/pmImgCreator.py
PMGENPMFEATURES := ../../tools/pmGenPmFeatures.py
IPM = true
DEBUG = true

TARGET = main
SOURCES = $(TARGET).c plat.c $(TARGET)_nat.c $(TARGET)_img.c
OBJS = $(SOURCES:.c=.o)

CINCS = -I$(abspath .) -I../../vm
ifeq ($(DEBUG),true)
	CDEBUGS += -g -D__DEBUG__=1
else
    CDEBUGS += -Os
endif
CFLAGS = -Wall -fno-strict-aliasing -Wstrict-prototypes \
         -Wdeclaration-after-statement -Werror $(CDEBUGS) $(CINCS)

export CFLAGS IPM PM_LIB_FN


.PHONY: all clean

all : pmfeatures.h $(TARGET).out

$(PM_LIB_PATH) : ../../vm/*.c ../../vm/*.h
	make -C ../../vm

$(TARGET).out : $(OBJS) $(PM_LIB_PATH)
	$(CC) -lm -o $@ $(OBJS) $(PM_LIB_PATH)

pmfeatures.h : pmfeatures.py $(PMGENPMFEATURES)
	$(PMGENPMFEATURES) pmfeatures.py > $@

# Generate native code and module images from the python source
$(TARGET)_nat.c $(TARGET)_img.c: $(PM_USR_SOURCES) pmfeatures.py
	$(PMIMGCREATOR) -f pmfeatures.py -c -u -o $(TARGET)_img.c --native-file=$(TARGET)_nat.c $(PM_USR_SOURCES)

clean :
	$(MAKE) -C ../../vm clean
	$(RM) $(TARGET).out $(OBJS) $(TARGET)_img.* $(TARGET)_nat.* pmfeatures.h
