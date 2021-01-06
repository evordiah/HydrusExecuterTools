CXX           = g++
CXXFLAGS      = -pipe -g -std=c++17 -Wall -W -D_REENTRANT -fPIC 
INCPATH       = -I. -I../thirdPartyLib  -I../thirdPartyLib/pqxx/include -I../HydrusFiles
LINK          = g++
LFLAGS        = 
SOURCES       = $(filter-out winpro%.cpp , $(wildcard *.cpp))
OBJECTS       = $(subst .cpp,.o,$(SOURCES))
LIBS          = -L../lib -lHydrusFiles -lpq -lpqxx -lstdc++fs -lpthread -ltinyxml2 -lb64
DEL_FILE      = rm -f 
DEL_DIR       = rm -fR 
MAKE_TARGET   = HydrusExecuter
DESTDIR       = ../bin/Linux/
#../bin/Linux/
TARGET        = $(DESTDIR)$(MAKE_TARGET)

.PHONY: all
all:  $(TARGET) clean

$(TARGET):  $(OBJECTS)  
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS)  $(LIBS)

clean:  
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core
	-$(DEL_FILE) $(wildcard *.d)
distclean: clean 
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_DIR) $(DESTDIR)
####### Compile
-include $(subst .cpp,.d,$(SOURCES))
%.d:%.cpp
	$(CXX) -c $(CXXFLAGS) -MM -MT "$*.o $@" $(INCPATH) $< >$@
%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

