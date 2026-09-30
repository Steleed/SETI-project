CC ?= gcc
CPPFLAGS += -Iinclude
LDLIBS += -pthread

BUILD_DIR := build
CLIENT_SOURCES := client/client.c client/client_aux.c
SERVER_SOURCES := server/server.c server/server_aux.c

.PHONY: all client server clean

all: client server

client: $(BUILD_DIR)/client

server: $(BUILD_DIR)/server

$(BUILD_DIR)/client: $(CLIENT_SOURCES) client/client.h include/project.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(CLIENT_SOURCES) $(LDLIBS)

$(BUILD_DIR)/server: $(SERVER_SOURCES) server/server.h include/project.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(SERVER_SOURCES) $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
