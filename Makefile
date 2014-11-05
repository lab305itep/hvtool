all : hvtool hvrstool hv hvprog

hvprog: hvprog.o hvop.o hvrslib.o
	gcc $^ -lm -o $@

hvrstool: hvrstool.o hvrslib.o
	gcc $^ -lreadline -o $@

hvtool : hvtool.o moc_hvtool.o hvop.o hvrslib.o
	g++ $^ -o $@ -lQtCore -lQtGui

hv : hv.o moc_hv.o hvop.o hvrslib.o
	g++ $^ -o $@ -lQtCore -lQtGui

moc_%.cpp: %.h
	moc-qt4 $< -o $@

clean :
	rm -f *.o ui_*.h moc_*.cpp
	rm -f hvtool

