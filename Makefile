
test: a.out
	./$<
	@echo Test successful

a.out: test_dangerous_buffer.cpp Makefile
	g++ $(CFLAGS) -Wall -Wextra -fsanitize=address -fsanitize=null -o $@ $< -DPAGE_SIZE=`getconf PAGE_SIZE`

clean:
	rm -rf a.out
