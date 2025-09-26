CC = gcc
CFLAGS = -g -I. -Wall -Wextra
LDFLAGs = -lm
DEPS = fft_functions.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ @< (CFLAGS)

FFT: fft_test.o fft_input_aaryan.o
	$(CC) -o fft_run fft_test.o fft_input_aaryan.o $(LDFLAGs) $(CFLAGS)

clean:
	$(RM) fft_test.o fft_input_aaryan.o