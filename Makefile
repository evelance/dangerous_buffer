
test: a.out
	./$<
	@echo Test successful

a.out: test_dangerous_buffer.cpp Makefile
	g++ $(CFLAGS) -Wall -Wextra -fsanitize=leak -fsanitize=address -fsanitize=null -o $@ $<

clean:
	rm -rf a.out
