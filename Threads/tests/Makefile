CC  = gcc
AR  = ar

CFLAGS = -g
LFLAGS = -lpthread

TGT = 	test \
	test_join \
	test_nested_join \
	test_many

OBJS =  ../pet_thread.o \
	../pet_thread_hw.o \
	../pet_hashtable.o \
	../pet_log.o 

build = \
	@if [ -z "$V" ]; then \
		echo '	[$1]	$@'; \
		$2; \
	else \
		echo '$2'; \
		$2; \
	fi

% : %.o
	$(call build,LINK,$(CC) $(CFLAGS) $(OBJS) $< -o $@ $(LFLAGS))

%.o : %.c 
	$(call build,CC,$(CC) $(CFLAGS) -c $< -o $@)

%.o : %.S 
	$(call build,CC,$(CC) $(CFLAGS) -c $< -o $@)

%.a : %.o
	$(call build,AR,$(AR) rcs $@ $^)

all: $(TGT)

test : $(OBJS) test.o

test_join : $(OBJS) test_join.o

test_nested_join : $(OBJS) test_nested_join.o

test_many : $(OBJS) test_many.o

clean: 
	rm -f *.o $(OBJS) $(TGT)
