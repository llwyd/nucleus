../bin/homeAssistant: sensor.o fsm.o daemon.o
	gcc -g -lm sensor.o fsm.o daemon.o -o ../bin/homeAssistant
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o
fsm.o: fsm.c
	gcc -c -g fsm.c -o fsm.o
sensor.o: sensor.c
	gcc -c -g sensor.c -o sensor.o
clean:
	rm ../bin/homeAssistant daemon.o fsm.o sensor.o
