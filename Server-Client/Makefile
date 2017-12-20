C = gcc

all: rsh rshd

rsh : rsh.o
        $(CC) $< -o $@

rshd : rshd.o
        $(CC) $< -o $@

%.o : %.c
        $(CC) -c $< -o $@

clean:
        rm -rf rsh rshd *.o *.swp *~
