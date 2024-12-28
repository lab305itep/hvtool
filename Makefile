CPPFLAGS=-I/usr/include/x86_64-linux-gnu/qt5 -fPIC
CFLAGS=-I/usr/include/x86_64-linux-gnu/qt5 -fPIC
LDFLAGS=-L/usr/lib/x86_64-linux-gnu/

all : hvtool hvrstool hv hvprog hvcmd

Release: all

hvprog: hvprog.o hvop.o hvrslib.o
	gcc $^ -lm -o $@

hvcmd: hvcmd.o hvop.o hvrslib.o
	gcc $^ -lm -o $@

hvrstool: hvrstool.o hvrslib.o
	gcc $^ -lreadline -o $@

hvtool : hvtool.o moc_hvtool.o hvop.o hvrslib.o
	g++ $^ -o $@ -lQt5Core -lQt5Gui -lQt5Widgets

hv : hv.o moc_hv.o hvop.o hvrslib.o
	g++ $^ -o $@ -lQt5Core -lQt5Gui -lQt5Widgets

moc_%.cpp: %.h
	moc $< -o $@

clean :
	rm -f *.o ui_*.h moc_*.cpp
	rm -f hvtool

