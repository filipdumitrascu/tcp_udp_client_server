# 323CA Dumitrascu Filip-Teodor
SOURCES_SERVER=server.cpp lib/common.cpp lib/helpers.cpp
SOURCES_TCP_CLIENT=tcp_client.cpp lib/common.cpp
CFLAGS=-c -Wall -Werror -Wno-error=unused-variable
INCPATHS=include
LIBPATHS=.
CC=g++

# Listening port
PORT = 12345

# Server IP address
IP_SERVER = 127.0.0.1

# Automatic generation of some important lists
OBJECTS_SERVER=$(SOURCES_SERVER:.cpp=.o)
OBJECTS_TCP_CLIENT=$(SOURCES_TCP_CLIENT:.cpp=.o)
INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

all: server subscriber

.cpp.o:
	$(CC) $(INCFLAGS) $(CFLAGS) -fPIC $< -o $@

server: $(OBJECTS_SERVER)
	$(CC) $(LIBFLAGS) $(OBJECTS_SERVER) -o server

subscriber: $(OBJECTS_TCP_CLIENT)
	$(CC) $(LIBFLAGS) $(OBJECTS_TCP_CLIENT) -o subscriber

clean:
	rm -rf server subscriber $(OBJECTS_SERVER) $(OBJECTS_TCP_CLIENT)

run_server: all
	./server ${PORT}

run_tcp: all    # make run_tcp ID=string (max 10 chars)
	./subscriber ${ID} ${IP_SERVER} ${PORT}
