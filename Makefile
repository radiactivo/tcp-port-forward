portforward: portforward.c
	$(CC) $(CCOPTS) $^ -o $@
debug: portforward.c
	$(CC) $(CCOPTS) -g $^ -o $@
clean:
	rm -f portforward debug