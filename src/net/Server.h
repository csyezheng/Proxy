//
// Created by csyezheng on 11/22/16.
//

#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <unistd.h>

namespace Proxy
{
    template<typename Protocol, typename IOMulti = Epoll>
    class Server
    {
    public:
        Server(unsigned port, int num);
        Server(const Server&) = delete;
        Server &operator= (const Server&) = delete;
        ~Server();

        unsigned port() const;
        void start();

    private:
        /* create, bind and listen socketfd, then return it*/
        int setup(unsigned port);

        int accept(int fd, struct sockaddr *addr, socklen_t *len);

        void listen_channel(int listenfd, EventLoop<IOMulti> *eventloop);

        void thread_pool();
        void loop(EventLoop<IOMulti>* eventloop);
        const unsigned ipPort;
        int MaxThreads;
        int sockfd;
        std::mutex amutex;
        std::vector<EventLoop<IOMulti>*> eventloops;
        std::map<Eventloop<IOMulti>*, std::list<Channel*>> closing_list;
    };
}
#endif //PROXY_SERVER_H