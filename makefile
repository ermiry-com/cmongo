TYPE		:= development

COVERAGE	:= 0

DEBUG		:= 0

SLIB		:= libcmongo.so

all: directories $(SLIB)

directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

install: $(SLIB)
	install -m 644 ./bin/libcmongo.so /usr/local/lib/
	cp -R ./include/cmongo /usr/local/include

uninstall:
	rm /usr/local/lib/libcmongo.so
	rm -r /usr/local/include/cmongo

MONGOC 		:= -l bson-1.0 -l mongoc-1.0
MONGOC_INC	:= -I /usr/local/include/libbson-1.0 -I /usr/local/include/libmongoc-1.0

DEFINES		:= -D _GNU_SOURCE

DEVELOPMENT := -D CMONGO_DEBUG

CC          := gcc

GCCVGTEQ8 	:= $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 8)

SRCDIR      := src
INCDIR      := include

BUILDDIR    := objs
TARGETDIR   := bin

SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

COVDIR		:= coverage
COVEXT		:= gcov

# common flags
# -Wconversion -march=native
COMMON		:= -Wall -Wno-unknown-pragmas \
				-Wfloat-equal -Wdouble-promotion -Wint-to-pointer-cast -Wwrite-strings \
				-Wtype-limits -Wsign-compare -Wmissing-field-initializers \
				-Wuninitialized -Wmaybe-uninitialized -Wempty-body \
				-Wunused-parameter -Wunused-but-set-parameter -Wunused-result \
				-Wformat -Wformat-nonliteral -Wformat-security -Wformat-overflow -Wformat-signedness -Wformat-truncation

# main
CFLAGS      := $(DEFINES)

ifeq ($(TYPE), development)
	CFLAGS += -g -fasynchronous-unwind-tables $(DEVELOPMENT)
else ifeq ($(TYPE), test)
	CFLAGS += -g -fasynchronous-unwind-tables -D_FORTIFY_SOURCE=2 -fstack-protector -O2
	ifeq ($(COVERAGE), 1)
		CFLAGS += -fprofile-arcs -ftest-coverage
	endif
else ifeq ($(TYPE), beta)
	CFLAGS += -g -D_FORTIFY_SOURCE=2 -O2
else
	CFLAGS += -D_FORTIFY_SOURCE=2 -O2
endif

# check which compiler we are using
ifeq ($(CC), g++) 
	CFLAGS += -std=c++11 -fpermissive
else
	CFLAGS += -std=c11 -Wpedantic -pedantic-errors
	# check for compiler version
	ifeq "$(GCCVGTEQ8)" "1"
    	CFLAGS += -Wcast-function-type
	else
		CFLAGS += -Wbad-function-cast
	endif
endif

# common flags
CFLAGS += -fPIC $(COMMON)

LIB         := -L /usr/local/lib $(MONGOC)

ifeq ($(TYPE), test)
	ifeq ($(COVERAGE), 1)
		LIB += -lgcov --coverage
	endif
endif

INC         := -I $(INCDIR) -I /usr/local/include $(MONGOC_INC)
INCDEP      := -I $(INCDIR)

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

SRCCOVS		:= $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(SRCEXT).$(COVEXT)))

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

$(SLIB): $(OBJECTS)
	$(CC) $^ $(LIB) -shared -o $(TARGETDIR)/$(SLIB)

# compile sources
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) $(LIB) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

# tests
TESTDIR		:= test
TESTBUILD	:= $(TESTDIR)/objs
TESTTARGET	:= $(TESTDIR)/bin
TESTCOVDIR	:= $(COVDIR)/test

TESTFLAGS	:= -g $(DEFINES) -Wall -Wno-unknown-pragmas -Wno-format

ifeq ($(TYPE), test)
	ifeq ($(COVERAGE), 1)
		TESTFLAGS += -fprofile-arcs -ftest-coverage
	endif
endif

TESTLIBS	:= $(PTHREAD) -L ./bin -l cmongo

TESTLIBS += -Wl,-rpath=./$(TARGETDIR) -L ./$(TARGETDIR) -l cmongo

ifeq ($(TYPE), test)
	ifeq ($(COVERAGE), 1)
		TESTLIBS += -lgcov --coverage
	endif
endif

TESTINC		:= -I $(INCDIR) -I ./$(TESTDIR)

TESTS		:= $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TESTOBJS	:= $(patsubst $(TESTDIR)/%,$(TESTBUILD)/%,$(TESTS:.$(SRCEXT)=.$(OBJEXT)))

TESTCOVS	:= $(patsubst $(TESTDIR)/%,$(TESTBUILD)/%,$(TESTS:.$(SRCEXT)=.$(SRCEXT).$(COVEXT)))

test: $(TESTOBJS)
	@mkdir -p ./$(TESTTARGET)
	@mkdir -p ./$(TESTTARGET)
	$(CC) $(TESTINC) ./$(TESTBUILD)/model.o -o ./$(TESTTARGET)/model $(TESTLIBS)
	$(CC) $(TESTINC) ./$(TESTBUILD)/select.o -o ./$(TESTTARGET)/select $(TESTLIBS)
	$(CC) $(TESTINC) ./$(TESTBUILD)/version.o -o ./$(TESTTARGET)/version $(TESTLIBS)

# compile tests
$(TESTBUILD)/%.$(OBJEXT): $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(TESTFLAGS) $(INC) $(TESTLIBS) -c -o $@ $<
	@$(CC) $(TESTFLAGS) $(INCDEP) -MM $(TESTDIR)/$*.$(SRCEXT) > $(TESTBUILD)/$*.$(DEPEXT)
	@cp -f $(TESTBUILD)/$*.$(DEPEXT) $(TESTBUILD)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(TESTBUILD)/$*.$(OBJEXT):|' < $(TESTBUILD)/$*.$(DEPEXT).tmp > $(TESTBUILD)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(TESTBUILD)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(TESTBUILD)/$*.$(DEPEXT)
	@rm -f $(TESTBUILD)/$*.$(DEPEXT).tmp

#coverage
COVOBJS		:= $(SRCCOVS) $(TESTCOVS)

test-coverage: $(COVOBJS)

coverage-init:
	@mkdir -p ./$(COVDIR)
	@mkdir -p ./$(TESTCOVDIR)

coverage: coverage-init test-coverage

# get lib coverage reports
$(BUILDDIR)/%.$(SRCEXT).$(COVEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p ./$(COVDIR)/$(dir $<)
	gcov -r $< --object-directory $(dir $@)
	mv $(notdir $@) ./$(COVDIR)/$<.gcov

# get tests coverage reports
$(TESTBUILD)/%.$(SRCEXT).$(COVEXT): $(TESTDIR)/%.$(SRCEXT)
	gcov -r $< --object-directory $(dir $@)
	mv $(notdir $@) ./$(TESTCOVDIR)

clear: clean-objects clean-tests clean-coverage

clean: clear
	@$(RM) -rf $(TARGETDIR)

clean-objects:
	@$(RM) -rf $(BUILDDIR)

clean-tests:
	@$(RM) -rf $(TESTBUILD)
	@$(RM) -rf $(TESTTARGET)

clean-coverage:
	@$(RM) -rf $(COVDIR)

.PHONY: all clean clear test coverage
