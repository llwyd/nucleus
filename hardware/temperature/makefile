../bin/temperature: comms.o sensor.o temp.o
	gcc -g comms.o sensor.o temp.o -o ../bin/temperature
temp.o: temp.c 
	gcc -c -g temp.c -o temp.o
sensor.o: sensor.c 
	gcc -c -g sensor.c -o sensor.o
comms.o: comms.c 
	gcc -c -g comms.c -o comms.o
clean:
	rm ../bin/temperature comms.o sensor.o temp.o
