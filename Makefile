SOURCE = crescent.c state/*.c types/*.c utils/*.c
OBJS   = crescent.o state/*.o types/*.o utils/*.h
HEADER = state/*.h  types/*.h utils/*.h
OUT    = crescent

FLAGS  = -Wall -Wextra -Wpedantic -c
LFLAGS =

all: $(OBJS)
	cc -g $(OBJS) -o $(OUT) $(LFLAGS)

crescent.o: crescent.c
	cc $(FLAGS) crescent.c -std=c99

state/*.o: state/*.c
	cc $(FLAGS) state/*.c -std=c99

types/*.o: types/*.c
	cc $(FLAGS) types/*.c -std=c99

clean:
	rm -f $(OBJS) $(OUT)
