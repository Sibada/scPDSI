EXPENDABLES = scpdsi
# LFLAGS = -mno-cygwin

scpdsi: scpdsi.cpp 
	g++ $(LFLAGS) -O scpdsi.cpp -g -o scpdsi

clean:
	rm -f $(EXPENDABLES)
	g++ $(LFLAGS) -O scpdsi.cpp -g -o scpdsi

all:
	rm -f $(EXPENDABLES)
	g++ $(LFLAGS) -O scpdsi.cpp -g -o scpdsi

# CC = g++
# SRCS := $(wildcard *.cpp)   
# OBJS := $(patsubst %cpp, %o, $(SRCS)) 

# echo $SRCS
# echo $OBJS

# TARGET = scpdsi
# all: $(TARGET)  
 
# $(TARGET): $(OBJS)  
# 	$(CC) $(CFLAGS) -o $@ $^
# %.o:%.cpp
# 	$(CC) $(CFLAGS) -c $<
 
# clean:  
# 	rm -f *.o $(TARGET)

# $@ 表示目标文件
# $^ 表示所有依赖项
# $< 表示第一个文件
