# 源文件列表
SRC = $(wildcard $(SRC_DIR)/*c)

# 目标文件列表
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

# 目标静态库名称
TARGET = $(BUILD_DIR)/libconfig.a

GFLAGS = -Wall -g -Iinc -I../../inc -I../log/inc
LIBS = -llog
LIBS_DIR = -L../log/build

# 链接静态库
$(TARGET): $(OBJ)
	ar -rcs $@ $(OBJ)
	$(CC) $(GFLAGS) $(OUT_LIBS_DIR) $(LIBS_DIR) -E $(SRC) $(LIBS) $(OUT_LIBS) -o chat_config.i 

# 编译源文件
# $(OBJ): $(SRC_DIR)/%.c
# 	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(GFLAGS) $(OUT_LIBS_DIR) $(LIBS_DIR) -c $< $(LIBS) $(OUT_LIBS) -o $@

# 清理
.PHONY: clean
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)