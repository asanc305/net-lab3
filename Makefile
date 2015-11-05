all:  sender receiver

sender: sender.c
	gcc -o sender sender.c -g

receiver: receiver.c
	gcc -o receiver receiver.c -g

clean:
	rm -f sender receiver *.o *~ core
