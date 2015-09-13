
CXX=g++
CXXFLAGS = -g -I./async_method/include -I/usr/include -I/usr/local/include -w -std=c++11
CXXLIBS  = -L/usr/lib -L/usr/local/lib -lboost_thread -lboost_system -lpthread

SOURCE_ALL = async_method/src/
TARGET_ALL = chatsystem

$(TARGET_ALL):	
	$(CXX) $(SOURCE_ALL)*.cpp  $(CXXFLAGS) $(CXXLIBS) -o  $(TARGET_ALL)

all:	$(TARGET_ALL)

clean:
	rm -f $(OBJS) $(TARGET_ALL) 
