BUILD_DIR ?= ./build
MKDIR_P ?= mkdir -p

CXXFLAGS += -std=c++17 -pedantic -Wall -Wextra -Wno-unused-parameter -O0 -g
INCLUDE += -I include
DEFINE +=
CPPFLAGS += $(DEFINE) $(INCLUDE) -MMD -MP

LIBCMINUS_SRC := $(shell find lib -name *.cpp -or -name *.c)
LIBCMINUS_OBJ := $(LIBCMINUS_SRC:%=$(BUILD_DIR)/%.o)
LIBCMINUS_DEP := $(LIBCMINUS_OBJ:.o=.d)

LEXICO_SRC := $(shell find src/lexico -name *.cpp -or -name *.c)
LEXICO_OBJ := $(LEXICO_SRC:%=$(BUILD_DIR)/%.o)
LEXICO_DEP := $(LEXICO_OBJ:.o=.d)

SINTATICO_SRC := $(shell find src/sintatico -name *.cpp -or -name *.c)
SINTATICO_OBJ := $(SINTATICO_SRC:%=$(BUILD_DIR)/%.o)
SINTATICO_DEP := $(SINTATICO_OBJ:.o=.d)

GERACODIGO_SRC := $(shell find src/geracodigo -name *.cpp -or -name *.c)
GERACODIGO_OBJ := $(GERACODIGO_SRC:%=$(BUILD_DIR)/%.o)
GERACODIGO_DEP := $(GERACODIGO_OBJ:.o=.d)


all: lexico sintatico geracodigo

libcminus.a: $(LIBCMINUS_OBJ)
	ar rcs $@ $(LIBCMINUS_OBJ)

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
	$(RM) lexico
	$(RM) sintatico
	$(RM) geracodigo

test:
	$(SHELL) -c 'cd test; . ./test.sh'

tidy:
	clang-tidy $(LIBCMINUS_SRC) $(CMINUS_SRC) $(LEXICO_SRC) \
		$(SINTATICO_SRC) $(GERACODIGO_SRC) -- $(INCLUDE) $(DEFINE) $(CXXFLAGS)

format:
	find -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \
		| xargs clang-format -i

-include $(LIBCMINUS_DEP)
-include $(LEXICO_DEP) 
-include $(SINTATICO_DEP)
-include $(GERACODIGO_DEP)
-include $(CMINUS_DEP)

.PHONY: all clean test tidy format
