GFLAGS = -Wall -g -Iinc -I../inc -I../lib/config_parser/inc

# 源文件列表
SRC = $(wildcard $(SRC_DIR)/*c)

# 目标文件列表
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

LIB_DIR = -L../lib/config_parser/build/

LIBS = -lconfig

TARGET = $(BIN_DIR)/server

all: $(TARGET)

# 生成目标文件
$(TARGET): $(OBJ)
	$(CC) $(GFLAGS) $(OUT_LIBS_DIR) $(LIB_DIR) $^ $(LIBS) $(OUT_LIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(GFLAGS) $(OUT_LIBS_DIR) $(LIB_DIR) -c $< $(LIBS) $(OUT_LIBS) -o $@

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*.o
