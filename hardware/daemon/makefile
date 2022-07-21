../bin/homeAssistant: sensor.o mqtt.o fsm.o daemon.o
	gcc -g -lm sensor.o mqtt.o fsm.o daemon.o -o ../bin/homeAssistant
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o -I../../stateengine/src
fsm.o: ../../stateengine/src/fsm.c
	gcc -c -g ../../stateengine/src/fsm.c -o fsm.o
mqtt.o: mqtt.c
	gcc -c -g mqtt.c -o mqtt.o
sensor.o: sensor.c
	gcc -c -g sensor.c -o sensor.o
clean:
	rm ../bin/homeAssistant daemon.o fsm.o sensor.o mqtt.o
