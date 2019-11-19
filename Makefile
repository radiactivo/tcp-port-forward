portforward: portforward.c
	$(CC) $(CCOPTS) $^ -o $@ -lpthread
debug: portforward.c
	$(CC) $(CCOPTS) -g $^ -o $@ -lpthread
clean:
	rm -f portforward debug *.so *.o