.PHONY: arsenal

arsenal:
	bake arsenal/hw.c
	bake arsenal/exit.c
	cd arsenal/hibot/ && bake main.c
	cd arsenal/hibot/ && bake test.c
