CC            = gcc
CFLAGS        = -pipe -O3 -Wall -W -D_REENTRANT -fPIC
CXX           = g++
CXXFLAGS      = -pipe -g -std=c++17 -Wall -W -D_REENTRANT -fPIC 
INCPATH       = -I. 
SOURCES      := $(wildcard *.c) 
OBJECTS      := $(subst .c,.o,$(SOURCES))
AR            = ar cqs
DEL_FILE      =rm -f 
MAKE_TARGET   = libb64.a
DESTDIR       =  ../../lib/
#../bin/Linux/
TARGET        = $(DESTDIR)$(MAKE_TARGET)


all:  $(TARGET) clean
.PHONY: all

$(TARGET):  $(OBJECTS)  
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(AR) $(TARGET) $(OBJECTS)

clean:  
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) $(wildcard *.d)

distclean: clean 
	-$(DEL_FILE) $(TARGET) 

####### Compile
-include $(subst .c,.d,$(SOURCES))
%.d:%.c
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -MM -MT "$*.o" $< >$@
%.o:%.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $< 
