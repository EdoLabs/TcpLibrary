/* 
 * Copyright (c) 2017 Ed Alegrid <ealegrid@gmail.com>
 * GNU General Public License v3.0
 */

# C++ Tcp Library

You can use the Tcp socket library as a general purpose networking interface for any
simple lightweight low payload client/server applications. 

The socket for both client and server is set to non-blocking rendering all subsequent read/send operations as non-blocking. 
The read buffer size is fixed to 1024 bytes which you can easily adjust to meet your requirements.

### Usage

Use any Linux C++11 compliant compiler or IDE to try it.

Using GCC g++, create the object file in the root folder where the main.cpp file is located.
For LLVM clang, just replace g++ with clang++ using the same options and following the same steps.

~~~
$ g++ -Wall -std=c++11 -c main.cpp -o obj/main.o
~~~
Then create the main executable. 
~~~
$ g++ -o bin/main obj/main.o -pthread
~~~
Run the application.
~~~
$ ./bin/main
~~~

### License
GNU General Public License v3.0

 
