
database:
	cc	database.c	-o	database

program:
	cc	program.c	-o	program

kaydet:
	cc	kaydet.c	-o	kaydet
clean:
	rm	-f	program	database	kaydet	sonuc.txt

all:	database	program	kaydet
