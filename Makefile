CXX ?= g++-7
BUILD_DIR ?= ./build
MKDIR_P ?= mkdir -p

CXXFLAGS += -std=c++17 -pedantic -Wall -Wextra -O0 -g
INCLUDE += -I include
DEFINE +=
CPPFLAGS += $(DEFINE) $(INCLUDE) -MMD -MP

LIBCMINUS_SRC := $(shell find lib -name *.cpp -or -name *.c)
LIBCMINUS_OBJ := $(LIBCMINUS_SRC:%=$(BUILD_DIR)/%.o)
LIBCMINUS_DEP := $(OBJS:.o:.d)

LEXICO_SRC := $(shell find src/lexico -name *.cpp -or -name *.c)
LEXICO_OBJ := $(LEXICO_SRC:%=$(BUILD_DIR)/%.o)
LEXICO_DEP := $(OBJS:.o:.d)

SINTATICO_SRC := $(shell find src/sintatico -name *.cpp -or -name *.c)
SINTATICO_OBJ := $(SINTATICO_SRC:%=$(BUILD_DIR)/%.o)
SINTATICO_DEP := $(OBJS:.o:.d)

GERACODIGO_SRC := $(shell find src/geracodigo -name *.cpp -or -name *.c)
GERACODIGO_OBJ := $(GERACODIGO_SRC:%=$(BUILD_DIR)/%.o)
GERACODIGO_DEP := $(OBJS:.o:.d)

CMINUS_SRC := $(shell find src/cminus -name *.cpp -or -name *.c)
CMINUS_OBJ := $(CMINUS_SRC:%=$(BUILD_DIR)/%.o)
CMINUS_DEP := $(OBJS:.o:.d)


all: lexico sintatico geracodigo cminus

libcminus.a: $(LIBCMINUS_OBJ)
	ar rcs $@ $(LIBCMINUS_OBJ)

cminus: $(CMINUS_OBJ) libcminus.a
	$(CXX) $(CMINUS_OBJ) -o $@ -L. -lcminus

lexico: $(LEXICO_OBJ) libcminus.a
	$(CXX) $(LEXICO_OBJ) -o $@ -L. -lcminus

sintatico: $(SINTATICO_OBJ) libcminus.a
	$(CXX) $(SINTATICO_OBJ) -o $@ -L. -lcminus

geracodigo: $(GERACODIGO_OBJ) libcminus.a
	$(CXX) $(GERACODIGO_OBJ) -o $@ -L. -lcminus


$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) libcminus.a
	$(RM) cminus
	$(RM) lexico
	$(RM) sintatico
	$(RM) geracodigo

tidy:
	clang-tidy $(LIBCMINUS_SRC) $(CMINUS_SRC) $(LEXICO_SRC) \
		$(SINTATICO_SRC) $(GERACODIGO_SRC) -- $(INCLUDE) $(DEFINE)

format:
	clang-format -i $(LIBCMINUS_SRC) $(CMINUS_SRC) $(LEXICO_SRC) \
		$(SINTATICO_SRC) $(GERACODIGO_SRC)

-include $(LIBCMINUS_DEP)
-include $(LEXICO_DEP) 
-include $(SINTATICO_DEP)
-include $(GERACODIGO_DEP)
-include $(CMINUS_DEP)

.PHONY: all clean tidy format
