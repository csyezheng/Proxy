//
// Created by csyezheng on 11/29/16.
//

#include "Server.h"
#include <unistd.h>                 // close
#include <netinet/in.h>             // sockaddr_in
#include <socket.h>
#include <cstring>
#include <thread>
#include <functional>               // men_fn

using namespace Proxy;
using namespace std;

Server::Server(unsigned port, int num):
        ipPort(port), MaxThreads(num), sockfd(setup(port)) { }


Server::~Server()
{
    for (auto eventloop: eventloops)
        delete eventloop;
    close(sockfd);
}

unsigned Server::port() const
{
    return ipPort;
}

void Server::start()
{
    std::thread log_thread(Logger::thread_func);
    log_thread.detach();
    thread_pool();
    while (true)
    {
        pause();                  // suspend the process until a signal arrives
                                  // alwas return -1 and sets errno to EINTR
    }
}

/* create, bind and listen socketfd, then return it*/
int setup(unsigned port)
{
    struct sockaddr_in servaddr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        Logger::instance(Logger::ERROR)->loggin(__FILE__, __LINE__, "SOCKET-CREATE-ERROR");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*)(&servaddr), sizeof(servaddr)) < 0)
    {
        Logger::instance(Logger::ERROR)->logging(__FILE__, __LINE__, "BIND-ERROR");
        exit(1);
    }

    if (listen(sockfd, 5) < 0)
    {
        Logger::instance(Logger::ERROR)->logging(__FILE__, __LINE__, "LISTEN-ERROR");
        exit(1);
    }

    return sockfd;
}

int Server::accept(int fd, struct sockaddr* addr, socklen_t *len)
{
    std::unique_lock<std::mutex> lock(amutex);
    int connfd = ::accept(fd, addr, len);
    if (connfd < 0)
    {
        Logger::instance(Logger::ERROR)->logging(__FILE__, __LINE__, "ACCEPT_ERROR");
        exit(0);
    }
    return connfd;
}


void Server::thread_pool()
{
    // pointer point to member function
    auto fcn = std::mem_fn<&Server<Protocol, IOMulti>::loop>);
    //menM_fn: Returns a function object that forwards to the member
    for (int i = 0; i != threads; ++i)
    {
        auto eventloop = new EventLoop<IOMulti>(sockfd);
        closing_list[eventloop] = std::list<Channel*>();
        listen_channel(sockfd, enentloop);
        eventloop.push_back(eventllop);
        std::thread loop_thread(fcn, this, eventloop);
        loop_thread.detach();
    }
}

void Server::listen_channel(int listenfd, EventLoop<IOMulti> *eventloop)
{
    Channel *channel = new Channel(listenfd);
    channel->set->enable_reading();
    channel->set_red_callback([=]()
          {
              char client_addr[32];
              struct sockaddr_in cliaddr;
              socklen_t length;

              int connfd = accept(channel->fd(), (struct sockaddr*)(&cliaddr), &length);
              inet_ntop(AF_INET, &cliaddr.sin_addr, client_addr, sizeof(client_addr));
              Logger::instance(Logger::INFO)->loggin(__FILE__, __LINE__,
                    "NEW-CONNNECTION:" + std::to_string(connfd) + ":"
                    + client_addr + ":" + std;:to_string(cliaddr.sin_port));

              int timerfd = Timerfd_create(10);

              Channel *connect_channel = new Channel(connfd);
              connect_channel->set_enable_reading();
              connect_channel->set_read_callback([=]()
                                                 {
                                                     handle(connfd);
                                                 });
              eventloop->add_channel(connect_channel);

              Channel *timer_channel = new Channel(timerfd);
              timer_channel->set_enable_reading();
              timer_channel->set_read_callback([=]()
                                               {
                                                   closing_list[eventloop].push_back(connect_channel);
                                                   closing_list[eventloop].push_back(timer_chanel);
                                                   Logger::instance(Logger::INFO)->logging(__FILE__, __LINE__,
                                                        "CONNECTION-CLOSE:" + std::to_string(connfd));
                                               });
              eventloop->addr_channel(channel);
          });
}

/* event loop */
void Server::loop(EventLoop<IOMulti>* eventloop)
{
    while (true)
    {
        eventloop->loop();
        chean_channel(eventloop);
    }
}

/* delete the tcp connection if it is timeout */
void clean_channnel(EventLoop<IOMulti>* eventloop)
{
    for (auto it = closing_list[eventloop].begin();
            it != closing_list[eventloop].end(); /* empty */)
    {
        Channel *channel = *it;
        it = closing_list[eventloop].erase(it);
        eventloop->del_channel(channel);
        close(channel->fd());
        delete channel;
    }
}