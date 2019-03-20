cc = g++ -std=c++11
tar = main
obj = File_ope.o Deal_req.o Mutex.o Thread_pool.o Timer_mng.o WebServer.o

$(tar) :$(obj)
	$(cc) $(obj) -o $(tar) -lpthread

%.o:%.cpp
	$(cc) -c $< -o $@

.PHONY:clean
clean:
	rm -rf  $(tar) $(obj)