sensorDaemon: ../common/sensor.o state.o daemon.o
	gcc -g ../common/sensor.o state.o daemon.o -o sensorDaemon
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o
state.o: state.c 
	gcc -c -g state.c -o state.o
../common/sensor.o: ../common/sensor.c
	gcc -c -g ../common/sensor.c -o ../common/sensor.o
clean:
	rm sensorDaemon state.o daemon.o ../common/sensor.o
