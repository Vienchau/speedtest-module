#CROSS_COMPILE =/home/vienchau/toolchain/HomeWifi/msdk-4.8.5-mips-EB-3.18-u0.9.33-m32ut-170828_lunapro-171213/msdk-4.8.5-mips-EB-3.18-u0.9.33-m32ut-170828_lunapro-171213/bin/mips-linux-uclibc-
CC = $(CROSS_COMPILE)gcc

LIBRARY_PATH =/home/vienchau/build/local/lib
INCLUDE_PATH = /home/vienchau/build/local/include

CFLAGS =-I$(INCLUDE_PATH) -s -O3 -fPIC \
	-pipe -fwrapv -fdata-sections  \
	-ffunction-sections -finline-functions \
	
	
LDFLAGS =-L$(LIBRARY_PATH) -lpthread -lcurl -lexpat -lm -lssl -lcrypto 

speedtest_module: speedtest.c
	$(CC) $< $(LDFLAGS)  $(CFLAGS) -o $@

clean:
	rm speedtest_module