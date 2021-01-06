SUBDIRS	= thirdPartyLib/b64 thirdPartyLib/pqxx thirdPartyLib/tinyxml2 HydrusFiles HydrusDBIO HydrusDBSetup HydrusExecuter
.PHONY: all clean
all: 
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir all; \
	done
	
clean:
	@for dir in ${SUBDIRS}; do \
		$(MAKE) -C $$dir clean; \
	done
