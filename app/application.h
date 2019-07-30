/*
* File:   Controller.h
* Author: Ed Alegrid
*
*/

#pragma once
#include <iostream>
#include <memory>
#include "../tcp/server.h"
#include "../tcp/client.h"
#include "device.h"

namespace project {

using namespace std;

class App
{
    public:
        App() {}
        ~App() {}

        /* application logic */

	      /*
      	 * webcontrol server for old web client (2 button control)
	       */
        void startCtrl()
        {
            cout << "\n*** C++ IO-Control Project ***\n" << endl;

            int serverport = 51111;
            int clientport = 5555;

            unique_ptr<Tcp::Server> server(new Tcp::Server);
            unique_ptr<Device:: ControlLogic> ControlModule(new  Device::ControlLogic);
            unique_ptr<Tcp::Client> client(new Tcp::Client);

            server->createServer(serverport);
            int loop = true;
            while(loop)
            {
                try
                {
                    // listen and accept new client
                    // set to true for continous loop, set to false or no parameter for one time server use
                    server->Listen(true);
                    // receive data from web client
                    string data = server->ReadAsync();
                    cout << "data from web client: " << data << endl;
                    // process received data
                    string msg = ControlModule->processData(data);
                    cout << "device status: " << msg << endl;
                    // tcp client connecting to server
                    // provide remote endpoint port and ip
                    // if ip is not provided it will default to localhost
                    client->Connect(clientport);
                    // send data to web client
                    client->SendAsync(data);
                    // close client socket
                    client->Close();
                    cout << "data to web client: " << data << endl;
                    cout << "waiting for new data ... \n\n";
                    // close server socket
                    // since Listen(true) is set to continous loop, close operation will only close the newsockfd but not sockfd
                    server->Close();
                }
                catch (SocketError& e)
                {
                    loop = false;
                    cerr << "error: " << e.what() << endl;
                    cout << "Please restart server." << endl;
                    server.reset(nullptr);
                    exit(1);
                }
            }
         server.reset(nullptr);
         exit(1);
        }

	      /*
      	 * client/server communication and non-blocking read timeout test
	       */
         void startTest()
        {
            cout << "\n*** C++ IO-Control Test ***\n" << endl;

            try
            {
                unique_ptr<Tcp::Server> server(new Tcp::Server(51111));
                unique_ptr<Tcp::Client> client(new Tcp::Client(51111));
                server->Listen();

                // client read should timeout since server didn't send any msg yet
                // the process should continue on the next operation
                client->ReadAsync(512); // non-blocking client read timeout

                client->SendAsync("Yo server, hi ya!");
                    cout << "data from client: " << server->ReadAsync() << endl;  
                server->SendAsync("Client bro, what's up!");
                    cout << "msg from server: " << client->ReadAsync() << endl;
                client->SendAsync("Server bro, I'm doin great!");
                    cout << "data from client: " << server->ReadAsync() << endl;

                // server read should timeout since client didn't send data after the first one
                server->ReadAsync(); // non-blocking server read async
                server->ReadAsync(); // non-blocking with server read timeout

                server->SendAsync("You're a lucky dude! See yah later, client bro!");
                    cout << "msg from server: " << client->ReadAsync() << endl;

                client->Close();
                // since Listen() is set to one time use w/o parameter
                // close operation will close both newsockfd and sockfd and stop the process
                server->Close();

            }
            catch (SocketError& e)
            {
                cerr << "error: " << e.what() << endl;
                exit(1);
            }

        exit(0);
        }

	     /*
      	* test using stack memory
	      */
        void startOtherTest()
        {
            cout << "\n*** C++ IO-Control Test using stack memory ***\n" << endl;

            using namespace Tcp;

            try
            {
                // client/server socket setup
                Server s;
                Client c;
                s.createServer(51111);
                c.Connect(51111);
                s.Listen();
                // client/server communication logic
                c.SendAsync("Yo server, hi ya!");
                cout << "client: " << s.ReadAsync() << endl;
                s.SendAsync("Client bro, what's up!");
                cout << "server: " << c.ReadAsync() << endl;
                c.SendAsync("Server dude, I'm doin fine!");
                cout << "client: " << s.ReadAsync() << endl;
                s.SendAsync("Great client bro, see yah later!");
                cout << "server: " << c.ReadAsync() << endl;
                // close and release sockets
                c.Close();
                s.Close();

            }
            catch (SocketError& e)
            {
                cerr << "error: " << e.what() << endl;
                exit(1);
            }

        exit(0);
        }

	     /*
        * simple echo server
	      */
        void startEchoServer()
        {
            cout << "\n*** C++ IO-Control Echo Server Demo ***\n" << endl;

            auto s = make_shared<Tcp::Server>(51111);

            for (;;)
            {
              try{
                s->Listen(true);
                cout << "received: " << s->SendAsync(s->ReadAsync()) << endl;
                s->Close();
              }
              catch (SocketError& e)
              {
                cerr << "error: " << e.what() << endl;
                exit(1);
              }
            }

       }
};

}
