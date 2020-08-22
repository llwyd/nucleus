sensorDaemon: state.o daemon.o
	gcc -g state.o daemon.o -o sensorDaemon
daemon.o: daemon.c 
	gcc -c -g daemon.c -o daemon.o
state.o: state.c 
	gcc -c -g state.c -o state.o
clean:
	rm sensorDaemon state.o daemon.o
