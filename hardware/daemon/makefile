sensorDaemon: ../common/comms.o ../common/sensor.o state.o daemon.o
	gcc -g -lm ../common/comms.o ../common/sensor.o state.o daemon.o -o sensorDaemon
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o
state.o: state.c 
	gcc -c -g state.c -lm -o state.o
../common/sensor.o: ../common/sensor.c
	gcc -c -g ../common/sensor.c -o ../common/sensor.o
../common/comms.o: ../common/comms.c
	gcc -c -g ../common/comms.c -o ../common/comms.o
clean:
	rm sensorDaemon state.o daemon.o ../common/sensor.o ../common/comms.o
