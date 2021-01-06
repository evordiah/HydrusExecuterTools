CXX           = g++
CXXFLAGS      = -pipe -g -std=c++17 -Wall -W -D_REENTRANT -fPIC 
INCPATH       = -I./include 
SOURCES      := $(wildcard src/*.cxx) 
OBJECTS      := $(subst .cxx,.o,$(SOURCES))
AR            = ar cqs
DEL_FILE      = rm -f 
MAKE_TARGET   = libpqxx.a
DESTDIR       =  ../../lib/
#../bin/Linux/
TARGET        = $(DESTDIR)$(MAKE_TARGET)

.PHONY: all
all:  $(TARGET) clean

$(TARGET):  $(OBJECTS)  
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(AR) $(TARGET) $(OBJECTS)

clean:  
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) $(wildcard src/*.d)

distclean: clean 
	-$(DEL_FILE) $(TARGET) 

####### Compile
-include $(subst .cxx,.d,$(SOURCES))
%.d:%.cxx
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -MM -MT "$*.o $@" $< >$@
%.o:%.cxx
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<
