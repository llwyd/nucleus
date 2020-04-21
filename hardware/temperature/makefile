../bin/temperature: sensor.o temp.o
	gcc sensor.o temp.o -o ../bin/temperature
temp.o: temp.c 
	gcc -c temp.c -o temp.o
sensor.o: sensor.c 
	gcc -c sensor.c -o sensor.o
clean:
	rm ../bin/temperature sensor.o temp.o