/*
 * Source File: server.h
 * Author: Ed Alegrid
 * Copyright (c) 2017 Ed Alegrid <ealegrid@gmail.com>
 * GNU General Public License v3.0
 */
#pragma once
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <thread>
#include <future>
#include <sstream>
#include "socketerror.h"

namespace Tcp {

using namespace std;

class Server
{
  int sockfd, newsockfd, PORT, rv;
  string IP;
  socklen_t clen;
  sockaddr_in server_addr{}, client_addr{};
  int listenF = false;
  int ServerLoop = false;
  struct pollfd rs[2];
 
  int initSocket(const int &port, const string ip = "127.0.0.1")
  {
    PORT = port;
    IP = ip;
    try
    {
	  if (port <= 0){
	    throw SocketError("Invalid port");
	  }
	  sockfd =  socket(AF_INET, SOCK_STREAM, 0);
	  if (sockfd < 0) {
	    throw SocketError();
	  }
	  server_addr.sin_family = AF_INET;
	  server_addr.sin_port = htons(port);
	  inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

	  int reuse = 1; //reuse socket
	  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	  if ( bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	  {
	    throw SocketError();
	  }
	  else
	  {
	    listen(sockfd, 5);
	    clen = sizeof(client_addr);
	  }
	  return 0;
    }
    catch (SocketError& e)
    {
	  cerr << "Server Socket Initialize Error: " << e.what() << endl;
	  closeHandler();
	  exit(1);
    }
  }

  void closeHandler() const
  {
    Close();
    delete this;
    exit(1);
  }

  public:
    // use with createServer() method
    Server(){}
    // immediately initialize the server socket with the port provided
    Server(const int &port, const string ip = "127.0.0.1" ): PORT{port}, IP{ip} { initSocket(port, ip); }
    virtual ~Server() {}

    void createServer(const int &port, const string ip = "127.0.0.1")
    {
        initSocket(port, ip);
    }

    void Listen(int serverloop = false)
    {
      ServerLoop = serverloop;
      try
      {
        auto l = [] (int fd, sockaddr_in client_addr, socklen_t clen)
        {
          auto newfd = accept4(fd, (struct sockaddr *) &client_addr, &clen, SOCK_NONBLOCK);
          if (newfd < 0){ throw SocketError("Invalid socket descriptor! Listen flag is false! \nMaybe you want to set it to true like Listen(true).");}
          return newfd;
        };


        if (!listenF){
          // initial server console output, provide one in the your application
          std::cout << "Server listening on: " << IP << ":" << PORT << "\n\n";
          listenF = true;
        }

        auto nfd = std::async(l, sockfd, client_addr, clen);
        newsockfd = nfd.get();

        //s td::cout << "server connection from client " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\n\n"; 
        rs[0].fd = newsockfd;
        rs[0].events = POLLIN | POLLPRI;

      }
      catch (SocketError& e)
      {
        std::cerr << "Server Listen Error: " << e.what() << std::endl;
        closeHandler();
      }
    }

    virtual const string Read()
    {
      char buffer[1024];
      try
      {
        if(!listenF){
          throw SocketError("No listening socket!\n Did you forget to start the Listen() method!");
        }

        // check socket event for available data, wait 100 milliseconds for timeout
        rv = poll(rs, 1, 10); // adjust timeout based on requirements
        if (rv < 0) {
          throw SocketError();
        } else if (rv == 0) {
          cout << "Server read timeout error! No data received!\n";
        } else {
          ssize_t n{1};
          bzero(buffer, sizeof(buffer));

          // check for events on newsockfd:
          if (rs[0].revents & POLLIN) {
            rs[0].revents = 0;
            n = {recv(newsockfd, buffer, sizeof(buffer), 0)}; // receive normal data
          }
          if (rs[0].revents & POLLPRI) {
            rs[0].revents = 0;
            n = {recv(newsockfd, buffer, sizeof(buffer), MSG_OOB)}; // out-of-band data
          }
          if (n == 0){
            cout << "Server read error, socket is closed or disconnected!\n";
          }
        }
      }
      catch (SocketError& e)
      {
        cerr << "Server Read Error: " << e.what() << endl;
        closeHandler();
      }
      return buffer;
    }

    // read data asynchronously, use only after calling Listen() method
    virtual const string ReadAsync(const int bufsize=1024) 
    //virtual const string ReadAsync()
    {
      // async data
      string ad;
      try
      {
        // lamda function
        auto l = [] (const int newsockfd, const int &bufsize=1024)
        {
          // cout << "server read async using lamda function" << endl; 
          string s;
          char buffer[bufsize];
          bzero(buffer, sizeof(buffer));
          ssize_t n {recv(newsockfd, buffer, sizeof(buffer), 0)};

          // cout << "server read async n bytes = " << n << endl; 
          if (n < 0) {
            cout << "Server read async error: No data available" << endl;
          }
          if (n == 0){
            cout << "Server read async error, socket at the other end is closed or disconnected!\n";
          }
          s = &buffer[0];
          // cout << "return buffer" << endl;
          return s; 
        };

        if(!listenF){
          throw SocketError("No listening socket!\n Did you forget to start the Listen() method!");
        }

        // check socket event for available data, wait 10 milliseconds for timeout
        rv = poll(rs, 1, 10); // adjust not higher than 10 ms for optimum wait time
        if (rv < 0) {
          throw SocketError();
        } else if (rv == 0) {
            cout << "Server read async timeout error! No data received!\n";
        } else {
          ssize_t n{1};
          if (rs[0].revents & POLLIN) {
          rs[0].revents = 0;
	  // return future data(fd) using inline lamda function
          auto rf = async(l, newsockfd, bufsize);
	  // async data 
          ad = rf.get();
          }
          if (n == 0){
            cout << "Server read async error, socket is closed or disconnected!\n";
          }
        }
      }
      catch (SocketError& e)
      {
        cerr << "Server Send Async Error: " << e.what() << endl;
        closeHandler();
      }
      return ad;
    }

    virtual const string Send(const string &msg) const
    {
        try
        {
          if(!listenF){
            throw SocketError("No listening socket!\n Did you forget to start the Listen() method!");
          }

          ssize_t n{send(newsockfd, msg.c_str(), strlen(msg.c_str()), 0)};
          if (n < 0) {
            throw SocketError();
          }
        }
        catch (SocketError& e)
        {
          cerr << "Server Send Error: " << e.what() << endl;
          closeHandler();
        }
        return msg;
    }

    // send data synchronously, use only after calling Listen() method
    virtual const string SendAsync(const string &msg) const
    {
      try
      {
        // using lambda expressions
        auto l = [] (const int newsockfd, const string &msg)
        {
    	// cout << "server send async using lambda function." << endl; 
    	ssize_t n{send(newsockfd, msg.c_str(), strlen(msg.c_str()), 0)};
    	if (n < 0) {
      		throw SocketError();
    	}
        return msg;
        };
        auto sf = std::async(l, newsockfd, msg); 
        auto d = sf.get();
          return d;
      }
      catch (SocketError& e)
      {
        cerr << "Server Async Send Error: " << e.what() << endl;
        closeHandler();
      }
      return msg;
    }

    virtual void Close() const
    {
        if(ServerLoop){
            close(newsockfd);
        }
        else{
            close(newsockfd);
            close(sockfd);
        }
    }
};

}
