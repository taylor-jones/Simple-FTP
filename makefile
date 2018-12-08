# Simple-FTP
# Master Makefile


.PHONY : build server client clean clean_server clean_client

# newline
define nl


endef


# 
# Build
# 

build: server client
		$(info All Done!${nl})

server:
		$(info Compiling FTP Server)
		@cd server && $(MAKE) -s

client:
		$(info Compiling FTP Client)
		@cd client && $(MAKE) -s


# 
# Clean
# 

clean: clean_server clean_client
		$(info All Clean!${nl})

clean_server:
		$(info Cleaning FTP Server)
		@cd server && $(MAKE) clean -s
	
clean_client:
		$(info Cleaning FTP Client)
		@cd client && $(MAKE) clean -s