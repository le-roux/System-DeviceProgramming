CFLAGS := -std=gnu99 -lpthread -Wall -Wextra -g
TARGET := file_concatenate
.PHONY : all clean

all : $(TARGET)

clean :
	rm -f *.o
	rm -f file_concatenate
	rm -f ./test/tmp/*
	rmdir ./test/tmp
	./foo.sh
