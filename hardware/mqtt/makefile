m: ../common/mqtt.o daemon.o
	gcc -g -lm ../common/mqtt.o daemon.o -o m
daemon.o: mqttDaemon.c 
	gcc -c -g mqttDaemon.c -o daemon.o
../common/mqtt.o: ../common/mqtt.c
	gcc -c -g -I../mqtt/ ../common/mqtt.c -o ../common/mqtt.o
clean:
	rm m daemon.o ../common/mqtt.o
