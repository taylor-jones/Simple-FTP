# Taylor Jones 
# Makefile - FTP Client

JFLAGS = -g
JC = javac
.SUFFIXES: .java .class
.java.class:
		$(JC) $(JFLAGS) $*.java

JVSRC = $(wildcard *.java)

default: classes

classes: $(JVSRC:.java=.class)

clean:
		$(RM) *.class