../bin/homeAssistant: fsm.o daemon.o
	gcc -g -lm fsm.o daemon.o -o ../bin/homeAssistant
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o
fsm.o: fsm.c
	gcc -c -g fsm.c -o fsm.o
clean:
	rm ../bin/homeAssistant daemon.o fsm.o 
