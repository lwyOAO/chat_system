#预定义变量
CC = gcc

# 源目录文件
SRC_DIR = src

# 输出文件目录
BUILD_DIR = build

# 引入的库文件
OUT_LIBS_DIR = -L/usr/lib/x86_64-linux-gnu -L/usr/lib
OUT_LIBS = -lyaml -luv -llog4c -lpthread -lncurses -lncursesw

# 程序目录
BIN_DIR = bin

SERVER = code/server/bin/server
CLIENT = code/client/bin/client

#传递预定义参数
export CC SRC_DIR BUILD_DIR BIN_DIR OUT_LIBS OUT_LIBS_DIR

.PHONY: all clean client server

# 编译所有组件
all: lib client server 
	mkdir -p $(BIN_DIR)/server $(BIN_DIR)/client $(BIN_DIR)/server/config $(BIN_DIR)/client/config $(BIN_DIR)/server/log $(BIN_DIR)/client/log
	mv $(SERVER) $(BIN_DIR)/server/
	mv $(CLIENT) $(BIN_DIR)/client/
	cp code/client/config/chat_config.yaml $(BIN_DIR)/client/config
	cp code/server/config/chat_config.yaml $(BIN_DIR)/server/config
	cp code/server/config/log4crc.in $(BIN_DIR)/server
	cp code/client/config/log4crc.in $(BIN_DIR)/client

# 进入client目录，调用 client 的 Makefile 进行编译
client: lib
	$(MAKE) -C code/client

server: lib
	$(MAKE) -C code/server

lib:
	$(MAKE) -C code/lib

clean:
	$(MAKE) -C client clean
	$(MAKE) -C server clean
	$(MAKE) -C lib clean
	rm -f /bin/*

