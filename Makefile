.PHONY: arsenal hibot

arsenal: arsenal/hw.out arsenal/exit.out hibot

hibot: arsenal/hibot/hibot.out arsenal/hibot/test.out

%.out: %.c
	${CC} ${CFLAGS} -o $@ $^ -std=c23 -lfcgi

clean:
	find arsenal/ -type f -name '*.out' -delete
