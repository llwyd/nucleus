../bin/temperature: ../common/comms.o sensor.o temp.o
	gcc -g ../common/comms.o sensor.o temp.o -o ../bin/temperature
temp.o: temp.c 
	gcc -c -g temp.c -o temp.o
sensor.o: sensor.c 
	gcc -c -g sensor.c -o sensor.o
../common/comms.o: ../common/comms.c 
	gcc -c -g ../common/comms.c -o ../common/comms.o
clean:
	rm ../bin/temperature ../common/comms.o sensor.o temp.o
