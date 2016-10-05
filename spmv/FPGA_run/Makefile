TARGET := $(MYOPENCL_HOST_CODE_FILE_NAME)_host

AOCL_COMPILE_CONFIG=$(shell aocl compile-config)
AOCL_LINK_CONFIG=$(shell aocl link-config | sed 's/lterasic/laltera/g')


all:
	g++ $(MYOPENCL_HOST_CODE_FILE_NAME).cpp my_spmv_common.cpp $(AOCL_COMPILE_CONFIG) $(AOCL_LINK_CONFIG) -o $(TARGET) -DVERBOSE=$(VERBOSE_LEVEL)

clean:
	rm $(TARGET)
