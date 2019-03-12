cc = g++
tar = main
obj = File_ope.o Deal_req.o Mutex.o Thread_pool.o WebServer.o

$(tar) :$(obj)
	$(cc) $(obj) -o $(tar)  -lpthread

clean:
	rm -rf  $(tar) $(obj)