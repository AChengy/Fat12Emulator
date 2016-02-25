all : flop help showfat structure showsector traverse showfile

flop : flop.c flop.h
	gcc -o flop flop.c flop.h -std=gnu99
	
help: help.c help.h
	gcc -o help help.c help.h -std=gnu99

showfat: showfat.c showfat.h
	gcc -o showfat showfat.c showfat.h -std=gnu99

structure: structure.c structure.h
	gcc -o structure structure.c structure.h -std=gnu99

showsector: showsector.c showsector.h
	gcc -o showsector showsector.c showsector.h -std=gnu99

traverse: traverse.h traverse.c
	gcc -o traverse traverse.c traverse.h -std=gnu99
	
showfile: showfile.c showfile.h
	gcc -o showfile showfile.c showfile.h -std=gnu99

.PHONY : clean
clean:
	rm -rf help flop showfat structure showsector traverse showfile
