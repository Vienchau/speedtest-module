ifdef cross
	CROSS_COMPILE =/home/vienchau/toolchain/HomeWifi/msdk-4.8.5-mips-EB-3.18-u0.9.33-m32ut-170828_lunapro-171213/msdk-4.8.5-mips-EB-3.18-u0.9.33-m32ut-170828_lunapro-171213/bin/mips-linux-uclibc-
	LIBRARY_PATH =/home/vienchau/build/fptw/lib
	INCLUDE_PATH = /home/vienchau/build/fptw/include
	LDFLAGS =-L$(LIBRARY_PATH) -lpthread -lcurl -lexpat -lm  -lssl -lcrypto 
	CFLAGS =-I$(INCLUDE_PATH) -s -O3 -fPIC -DCROSS\
	-pipe -fwrapv -fdata-sections  \
	-ffunction-sections -finline-functions 
else
	LDFLAGS =-L$(LIBRARY_PATH) -lpthread -lcurl -lexpat -lm 
	CFLAGS =-I$(INCLUDE_PATH) -s -O3 -fPIC  -fsanitize=address -g\
	-pipe -fwrapv -fdata-sections  \
	-ffunction-sections -finline-functions 
endif

CC = $(CROSS_COMPILE)gcc


speedtest_module: speedtest.c
	$(CC) $< $(LDFLAGS)  $(CFLAGS) -o $@

clean:
	rm speedtest_module