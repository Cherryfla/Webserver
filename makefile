cc = g++
tar = main
obj = File_ope.o Deal_req.o WebServer.o

$(tar) :$(obj)
	$(cc) -o $(tar) $(obj)

WebServer.o: WebServer.cpp
	$(cc) -c $< -o $@

Deal_req.o: Deal_req.cpp
	$(cc) -c $< -o $@

File_ope.o: File_ope.cpp
	$(cc) -c $< -o $@

clean:
	rm -rf  $(tar) $(prom)