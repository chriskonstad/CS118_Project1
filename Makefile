CC=g++
CFLAGS=-Wall -std=c++0x -g
LDFLAGS=
SOURCES=main.cpp server.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)

cleandoc:
	rm report.pdf

doc: report.pdf

report.pdf : doc/report.md
	pandoc doc/report.md -o report.pdf
