portforward: portforward.c
	$(CC) $(CCOPTS) -Wall $^ -o $@
debug: portforward.c
	$(CC) $(CCOPTS) -Wall -g $^ -o $@
clean:
	rm -f portforward debug *.so *.o