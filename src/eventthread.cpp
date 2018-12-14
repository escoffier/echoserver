#include "eventhread.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>

Eventthread::Eventthread(std::shared_ptr<ItemQueue> queue)
{
    itemQueue_ = queue;
}

Eventthread::~Eventthread()
{
    if(thread_.joinable())
    {
        thread_.join();
    }
}

bool Eventthread::init()
{
    base = event_base_new();
    if (!base)
    {
        std::cout << "Could not initialize libevent!\n";
        return false;
    }

    if(pipe(fds) == -1)
    {
        std::cout << "pipe err\n";
        return false;
    }

    notify_receive_fd = fds[0];
    notify_send_fd = fds[1];

    struct event *pipeEvent;
    std::cout << "notify_receive_fd : " << notify_receive_fd <<std::endl;
    pipeEvent = event_new(base, notify_receive_fd, EV_READ | EV_PERSIST, process_new_conn, this);
    event_add(pipeEvent, NULL);

    thread_ = std::thread(&Eventthread::run, this);
    std::cout << "Eventthread init OK\n";
    return true;
}

int Eventthread::addEvent(evutil_socket_t fd)
{
    std::cout << "Eventthread addEvent , fd: " << fd << std::endl;
    struct bufferevent *bufev = NULL;
    bufev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bufev)
    {
        std::cout << "Error constructing bufferevent!";
        event_base_loopbreak(base);
        return -1;
    }

    bufferevent_setcb(bufev, conn_readcb, conn_writecb, conn_eventcb, NULL);
    if (bufferevent_enable(bufev, EV_READ)  == -1)
    {
        std::cout << "bufferevent_enable error";
        return -1;
    }
    return 0;
    //bufferevent_disable(bufev, EV_READ);
}

void Eventthread::run()
{
    std::cout << "Eventthread dispatching" << std::endl;
    event_base_dispatch(base);
     std::cout << "Eventthread dispatch exit" << std::endl;
}

void Eventthread::process_new_conn(evutil_socket_t fd, short nevent, void *arg)
{
    Eventthread *evth = (Eventthread *)arg;
    std::cout << "Eventthread pipe event : " << nevent << std::endl;
    char buf[1];
    if (read(evth->getReadfd(), buf, 1) != 1)
    {
        std::cout << "read pipe err\n";
        return;
   }
   std::shared_ptr<Item> item;
   switch (buf[0])
   {
   case 'c':
       item = evth->itemQueue_->pop();
       if(item)
           evth->addEvent(item->fd);
        else
           std::cout << "itemQueue_ is empty\n";
       break;
   default:
       break;
    }
}

void Eventthread::conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF)
    {
        std::cout<< "Connection closed , fd: " << bufferevent_getfd(bev) <<std::endl;
    }
    else if (events & BEV_EVENT_ERROR)
    {
        printf("Got an error on the connection: %s\n",
               strerror(errno)); /*XXX win32*/
    }
    /* None of the other events can happen here, since we haven't enabled
	 * timeouts */
    bufferevent_free(bev);
}

void
Eventthread::conn_writecb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		printf("conn_writecb flushed answer\n");
		//bufferevent_free(bev);
	}
}

void Eventthread::conn_readcb(struct bufferevent *bev, void *user_data)
{
     std::cout << "Eventthread conn_readcb" << std::endl;
    char data[1024] = {0};
    struct evbuffer *input = bufferevent_get_input(bev);
    bufferevent_read(bev, data, evbuffer_get_length(input));

    std::cout << "receivce data: " << data << std::endl;
    std::string resp("hello, robbie");
    bufferevent_write(bev, resp.c_str(), resp.size());
}