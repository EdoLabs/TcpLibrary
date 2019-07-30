/*
 * File:   main.ccp
 * Author: Ed Alegrid
 *
 * Use any Linux C++11 compliant compiler or IDE.
 *
 *
 * This is a very simple TCP client/server library with non-blocking read timeout.
 * It was mainly use for testing TCP socket communications with Node-WebControl project.
 *
 */

#include <iostream>
#include <memory>
#include "app/application.h"

int main()
{
    std::unique_ptr<project::App> app(new project::App);

    /* run and test each method one at a time */
    //app->startCtrl(); // for 2 button web client only
    //app->startTest();
    //app->startOtherTest();
    app->startEchoServer();

   return 0;
}
