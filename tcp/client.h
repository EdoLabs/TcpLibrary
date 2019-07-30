/*
 * Source File: client.h
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
#include <sstream>
#include <netdb.h>
#include "socketerror.h"

namespace Tcp {

using namespace std;

class Client
{
    int sockfd, rv, rd;
    char s[INET6_ADDRSTRLEN];
    struct pollfd rs[1];

    void *get_addr(struct sockaddr *sa)
    {
      if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
      }
      return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

    int initSocket(int port, string ip)
    {
      string PORT;
      stringstream out;
      out << port;
      PORT = out.str();

      addrinfo hints{};
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      addrinfo *servinfo{};

      try{
        if (port <= 0){
          throw SocketError("Invalid port");
        }
        if ((rv = getaddrinfo(ip.c_str(), PORT.c_str(), &hints, &servinfo)) != 0) {
          cout << "getaddrinfo: " << gai_strerror(rv) << endl;
          throw SocketError("Invalid address");
        }
        sockfd = {socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)};
        int result{ connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)};
          if (result < 0)
          {
            cout << "Client connection failed!" << endl;
            throw SocketError();
          }
          else
          {
          // details of remote connected endpoint
          inet_ntop(servinfo->ai_family, get_addr((struct sockaddr *)servinfo->ai_addr), s, sizeof s);
          // initial client console output, provide one in your application
          cout << "Client connected to: " << s << ":" << port << endl; 
          }
          if (servinfo == nullptr) {
            throw SocketError("Client connect fail ...");
          }

          freeaddrinfo(servinfo);

          if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0){ throw SocketError();}

          rs[0].fd = sockfd;
          rs[0].events = POLLIN | POLLPRI;

        return 0;
      }
      catch (SocketError& e)
      {
        cerr << "Client Socket Initialize Error: " << e.what() << endl;
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
    // use with Connect() method
    Client() {}
    // immediately initialize the client socket with the port and ip provided
    Client(const int port, const string ip = "127.0.0.1")  {initSocket(port, ip);}
    virtual ~Client() {}

    virtual void Connect(const int port, const string ip = "127.0.0.1")
    {
      initSocket(port, ip);
    }

    virtual string Send(const string msg) const
    {
	try
	{
	  int n = send(sockfd, msg.c_str(), strlen(msg.c_str()), 0);
	  if (n < 0) {
	    throw SocketError();
	  }
	}
	catch (SocketError& e)
	{
	  cerr << "Client Send Error: " << e.what() << endl;
	  closeHandler();
	}
	return msg;
    }

    // send data synchronously, use only after calling Listen() method
    virtual const string SendAsync(const string &msg) const
    {
	try
	{
	  auto l = [] (const int sockfd, const string &msg)
	  {
	    // cout << "client send async using lambda function." << endl; 
	    ssize_t n{send(sockfd, msg.c_str(), strlen(msg.c_str()), 0)};
	    if (n < 0) {
	      throw SocketError();
	    }
	    return msg;
	  };

	  auto sf = std::async(l, sockfd, msg); 
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

    virtual const string Read()
    {
	char buffer[1024];
        try
        {
          // check socket event for available data, wait 200 milliseconds for timeout
          rd = poll(rs, 1, 10); //200
          if (rd < 0) {
            throw SocketError();
          }
          else if (rd == 0) {
            cout << "Client read timeout error! No data received!\n";
          }
          else {
            ssize_t n{1};
            bzero(buffer, sizeof(buffer));
            // check for events on newsockfd:
            if (rs[0].revents & POLLIN) {
              rs[0].revents = 0;
              n = {recv(sockfd, buffer, sizeof(buffer), 0)}; // received normal data
            }
            if (rs[0].revents & POLLPRI) {
              rs[0].revents = 0;
              n = {recv(sockfd, buffer, sizeof(buffer), MSG_OOB)}; // out-of-band data
            }
            if (n == 0){
              cout << "Client read error, socket is closed or disconnected!\n";
            }
          }
        }
        catch (SocketError& e)
        {
          cerr << "Client Read Error: " << e.what() << endl;
          closeHandler();
        }
        return buffer;
    }

    // virtual const string ReadAsync()
    virtual const string ReadAsync(int bufsize=1024) 
    {
	// async data
	string ad;
        try
        { 
          auto l = [] (const int sockfd, const int &bufsize=1024)
          {
            string s;
            char buffer[bufsize];
            bzero(buffer, sizeof(buffer));
            ssize_t n{recv(sockfd, buffer, sizeof(buffer), 0)};

            // cout << "client read async lamda n bytes = " << n << endl; 
            if (n < 0) {
              cout << "client read async lamda error: No data available" << endl;
            }
            if (n == 0){
              cout << "client read async lamda error, socket at the other end is closed or disconnected!\n";
            }
            s = &buffer[0];
            // cout << "return buffer" << endl;
            return s;
          };

          // check socket event for available data, wait 10 milliseconds for timeout
          rv = poll(rs, 1, 10); // adjust not higher than 10 ms for optimum wait time
          if (rv < 0) {
            throw SocketError();
          }
          else if (rv == 0) {
            cout << "client read async timeout error! No data received!\n";
          }
          else {
            ssize_t n{1};
            if (rs[0].revents & POLLIN) {
            rs[0].revents = 0;
	    // return future data(fd) using inline lamda function
            auto rf = async(l, sockfd, bufsize); 
            ad = rf.get();
            }
            if (n == 0){
              cout << "client read async error, socket is closed or disconnected!\n";
            }
          }
        }
        catch (SocketError& e)
        {
          cerr << "Client Read Async Error: " << e.what() << endl;
          closeHandler();
        }
        return ad;
    }

    virtual void Close() const
    {
        close(sockfd);
    }
};

}

