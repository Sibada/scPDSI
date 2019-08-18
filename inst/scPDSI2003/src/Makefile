# EXPENDABLES = scpdsi
# LFLAGS = -mno-cygwin

SRCS := $(wildcard *.cpp)   
OBJS := $(patsubst %cpp, %o, $(SRCS)) 

dir_dll="C:/Program Files/R/R-3.5.3/bin/x64"
# dir_dll="C:/Program Files/R/R-3.5.3/bin/i386"

dir_r="C:/Program Files/R/R-3.5.3/include"
dir_rcpp="D:/Documents/OneDrive - mail2.sysu.edu.cn/sync/R-library/3.6/Rcpp/include"
# echo $(SRCS)

CFLAGS = -w
# -Wwrite-strings
CC = g++ -I$(dir_rcpp) -I$(dir_r) -L$(dir_dll) -lR -g 
# -O2 -Wall -mtune=generic

# flags = -DNDEBUG -shared -s -static-libgcc
# scpdsi: scpdsi.cpp 
# 	g++ $(LFLAGS) -O scpdsi.cpp -g -o scpdsi

# clean:
# 	rm -f $(EXPENDABLES)
# 	g++ $(LFLAGS) -O scpdsi.cpp -g -o scpdsi

# all:
# 	rm -f $(EXPENDABLES)
# 	g++ $(LFLAGS) -O scpdsi.cpp -g -o scpdsi

TARGET = ../scpdsi
all: $(TARGET)

$(TARGET): $(OBJS)  
	$(CC) $(CFLAGS) $(flags) -o $@ $^
%.o:%.cpp
	$(CC) $(CFLAGS) -c $<
 
clean:  
	rm -f *.o $(TARGET)

# $@ 表示目标文件
# $^ 表示所有依赖项
# $< 表示第一个文件
