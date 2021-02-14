m: ../common/task.o ../common/mqtt.o daemon.o
	gcc -g -lm ../common/task.o ../common/mqtt.o daemon.o -o m
daemon.o: mqttDaemon.c 
	gcc -c -g mqttDaemon.c -o daemon.o
../common/mqtt.o: ../common/mqtt.c
	gcc -c -g -I../mqtt/ ../common/mqtt.c -o ../common/mqtt.o
../common/task.o: ../common/task.c
	gcc -c -g -I../mqtt/ ../common/task.c -o ../common/task.o
clean:
	rm m daemon.o ../common/mqtt.o
