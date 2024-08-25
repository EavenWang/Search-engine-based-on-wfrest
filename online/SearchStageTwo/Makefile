TARGET = a.out

OBJS = testWfrest.o  KeyRecommander.o SearchEngineServer.o WebPageQuery.o tinyxml2.o

CC = g++

$(TARGET):$(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -lm -lwfrest -lworkflow -I ../include

testWfrest.o:testWfrest.cc
	$(CC) -c testWfrest.cc -o testWfrest.o

KeyRecommander.o:../src/KeyRecommander.cc
	$(CC) -c ../src/KeyRecommander.cc -o KeyRecommander.o

SearchEngineServer.o:../src/SearchEngineServer.cc
	$(CC) -c ../src/SearchEngineServer.cc -o SearchEngineServer.o

WebPageQuery.o:../src/WebPageQuery.cc
	$(CC) -c ../src/WebPageQuery.cc -o WebPageQuery.o

tinyxml2.o:../../tinyxml/tinyxml2.cpp
	$(CC) -c ../../tinyxml/tinyxml2.cpp -o tinyxml2.o

clean:
	rm $(OBJS) $(TARGET)


 
