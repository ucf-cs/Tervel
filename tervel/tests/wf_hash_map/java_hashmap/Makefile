JFLAGS = -classpath .:high-scale-lib/lib/high-scale-lib.jar:high-scale-lib/lib/java_util_concurrent_chm.jar:high-scale-lib/lib/java_util_hashtable.jar
JC = javac
.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

CLASSES = HashMapTester.java

default: classes

classes: $(CLASSES:.java=.class)

clean:
	$(RM) *.class
	$(RM) Cliff.jar
jar:
	jar cvfm Cliff.jar manifest.mf *.class high-scale-lib/lib/high-scale-lib.jar high-scale-lib/lib/java_util_concurrent_chm.jar high-scale-lib/lib/java_util_hashtable.jar

run:
	java -Xmx500g $(JFLAGS) -cp Cliff.jar HashMapTester 1024 1 5 50 25 25 25 25

