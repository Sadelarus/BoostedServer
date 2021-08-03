DESTDIR ?= ${HOME}/Boost/
SOURCE1 = client.cpp
SOURCE2 = server.cpp

all: server client

server: ${SOURCE2}
	g++ ${SOURCE2} -g -o $@ -pthread

client: ${SOURCE1}
	g++ ${SOURCE1} -g -o $@ -pthread

clean:
	${RM} server
	${RM} client

instal: all
	mkdir -p ${DESTDIR}
	cp server ${DESTDIR}
	cp client ${DESTDIR}

.PHONY: all clean install
