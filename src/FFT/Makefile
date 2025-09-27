CC = gcc
CFLAGS = -g -Wall -Wextra 
CPPFLAGS  = -I. -I../../lib/kissfft-master
LDFLAGS = -lm
DEPS = fft_functions.h

#App sources
APP_SRC  = stft.c fft_input_aaryan.c

#KISS_FFT sources
KISS_DIR = ../../lib/kissfft-master
KISS_SRC = $(KISS_DIR)/kiss_fft.c $(KISS_DIR)/kiss_fftr.c

#Build sets
SRC   = $(APP_SRC) $(KISS_SRC)
OBJ   = $(SRC:.c=.o)
DEPD  = $(OBJ:.o=.d)  
HEADS = fft_functions.h       

.PHONY: clean always

all: fft

%.o: %.c $(DEPS) always
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

#Run FFT Tests
fft: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)
	./fft

#Remove executable files
clean:
	$(RM) stft.o fft_input_aaryan.o fft_run

-include $(DEPS)