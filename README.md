# Webserver
使用kqueue处理多响应的服务器。目前还只有一个雏形。

# 开发环境
Macos 10.14.3 Mojave

# 执行命令
make  && ./main

# 技术要点
+ 采用kqueue I/O多路复用
+ 使用小根堆实现定时器管理(待完善)
+ 使用线程池，提高了线程管理效率
+ 引入内存池，提高访存速度