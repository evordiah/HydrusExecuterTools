CXX           = g++
CXXFLAGS      = -pipe -g -std=c++17 -Wall -W -D_REENTRANT -fPIC 
INCPATH       = -I. -I../thirdPartyLib  -I../thirdPartyLib/pqxx/include
SOURCES      := $(wildcard *.cpp) 
OBJECTS      := $(subst .cpp,.o,$(SOURCES))
AR            = ar cqs
DEL_FILE      = rm -f 
DEL_DIR       = rm -fR
MAKE_TARGET   = libHydrusFiles.a
DESTDIR       =  ../lib/
#../bin/Linux/
TARGET        = $(DESTDIR)$(MAKE_TARGET)

.PHONY: all
all:  $(TARGET) clean

$(TARGET):  $(OBJECTS)  
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(AR) $(TARGET) $(OBJECTS)

clean:  
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) $(wildcard *.d)
distclean: clean 
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_DIR) $(DESTDIR)
####### Compile
-include $(subst .cpp,.d,$(SOURCES))
%.d:%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -MM -MT "$*.o $@" $< >$@
%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<
