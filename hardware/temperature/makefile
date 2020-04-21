../bin/temperature: comms.o sensor.o temp.o
	gcc comms.o sensor.o temp.o -o ../bin/temperature
temp.o: temp.c 
	gcc -c temp.c -o temp.o
sensor.o: sensor.c 
	gcc -c sensor.c -o sensor.o
comms.o: comms.c 
	gcc -c comms.c -o comms.o
clean:
	rm ../bin/temperature comms.o sensor.o temp.o