../bin/homeAssistant: sensor.o mqtt.o state.o daemon.o
	gcc -g -lm sensor.o mqtt.o state.o daemon.o -o ../bin/homeAssistant
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o -I../../stateengine/src
state.o: ../../stateengine/src/state.c
	gcc -c -g ../../stateengine/src/state.c -o state.o
mqtt.o: mqtt.c
	gcc -c -g mqtt.c -o mqtt.o
sensor.o: sensor.c
	gcc -c -g sensor.c -o sensor.o
clean:
	rm ../bin/homeAssistant daemon.o state.o sensor.o mqtt.o
