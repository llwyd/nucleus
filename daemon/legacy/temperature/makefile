../bin/temperature: ../common/comms.o ../common/sensor.o temp.o
	gcc -g ../common/comms.o ../common/sensor.o temp.o -o ../bin/temperature
temp.o: temp.c 
	gcc -c -g temp.c -o temp.o
../common/sensor.o: ../common/sensor.c 
	gcc -c -g ../common/sensor.c -o ../common/sensor.o
../common/comms.o: ../common/comms.c 
	gcc -c -g ../common/comms.c -o ../common/comms.o
clean:
	rm ../bin/temperature ../common/comms.o ../common/sensor.o temp.o
