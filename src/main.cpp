#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif
#include <sys/socket.h>
#endif


#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <iostream>
#include <unistd.h>

#include "eventhread.h"

const int PORT = 9995;
struct timeval lasttime;

int event_is_persistent;
std::shared_ptr<ItemQueue> itemqueue = std::make_shared<ItemQueue>();

static void
conn_writecb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");
		bufferevent_free(bev);
	}
}

static void conn_readcb(struct bufferevent *bev, void *user_data)
{
    char data[1024] = {0};
    struct evbuffer *input = bufferevent_get_input(bev);
    bufferevent_read(bev, data, evbuffer_get_length(input));
    std::cout << "receivce data: " << data << std::endl;

}


static void
conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	if (events & BEV_EVENT_EOF) {
		std::cout<< "Connection closed , fd: " << bufferevent_getfd(bev) <<std::endl;
	} else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
		    strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}

static const char MESSAGE[] = "Hello, World!\n";

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                        struct sockaddr *sa, int socklen, void *user_data)
{
    struct event_base *base = (event_base *)user_data;
    struct bufferevent *bufev = NULL;
    bufev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bufev)
    {
        std::cout<< "Error constructing bufferevent!";
        event_base_loopbreak(base);
        return;
    }

    bufferevent_setcb(bufev, NULL, conn_writecb, conn_eventcb, NULL);
    bufferevent_enable(bufev, EV_WRITE);
    bufferevent_disable(bufev, EV_READ);
 
    bufferevent_write(bufev, MESSAGE, strlen(MESSAGE));
}

static void listener_cb_new(struct evconnlistener *listener, evutil_socket_t fd,
                        struct sockaddr *sa, int socklen, void *user_data)
{
    std::cout << " listener new connection" << fd << std::endl;

    Eventthread *evthread = (Eventthread *)user_data;
    std::shared_ptr<Item> item = std::make_shared<Item>();
    item->fd = fd;
    item->flags = EV_READ | EV_WRITE;
    itemqueue->push(item);
   
    char buf[1];
    buf[0] = 'c';
     if (write(evthread->getWritefd(), buf, 1) != 1) {
         std::cout << "Writing to thread notify pipe";
     }
}

static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = (event_base *)user_data;
	struct timeval delay = { 1, 0 };

	std::cout << "Caught an interrupt signal; exiting cleanly in 1 seconds.\n";

    event_base_loopexit(base, &delay);
}

static void
timeout_cb(evutil_socket_t fd, short nevent, void *arg)
{
	struct timeval newtime, difference;
    struct event *timeeoutev = *(event **)arg;

    if(!timeeoutev)
    {
        std::cout << "timeeoutev is null";
    }

    double elapsed;

    evutil_gettimeofday(&newtime, NULL);
	evutil_timersub(&newtime, &lasttime, &difference);
	elapsed = difference.tv_sec +
	    (difference.tv_usec / 1.0e6);

	printf("timeout_cb called at %d: %.3f seconds elapsed.\n",
	    (int)newtime.tv_sec, elapsed);
	lasttime = newtime;

	if (! event_is_persistent) {
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 20;
		event_add(timeeoutev, &tv);
	}
}

int main(int argc, char const *argv[])
{
    /* code */
    struct event_base *base;
    struct evconnlistener *listener;
    struct event *signal_event;
    //struct event timeout;
    struct event *timeoutev;

    struct sockaddr_in sin;

    base = event_base_new();
    if (!base)
    {
        std::cout << "Could not initialize libevent!\n";
        return 1;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    
    Eventthread *evthread = new Eventthread(itemqueue);

    if(!evthread->init())
    {
        std::cout << "Could not create a Eventthread!\n";
        return 1;
    }
    
    listener = evconnlistener_new_bind(base, listener_cb_new, (void *)evthread,
                                       LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                       (sockaddr *)&sin, sizeof(sin));

    if (!listener)
    {
        std::cout << "Could not create a listener!\n";
        return 1;
    }

    signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

    if(!signal_event)
    {
        std::cout << "Could not create a signal!\n";
        return 1;
    }

    if(event_add(signal_event, NULL) < 0)
    {
        std::cout << "Could not add a signal!\n";
        return 1;
    }

    //event_assign(&timeout, base, -1, EV_PERSIST, timeout_cb, (void *)&timeout);
    timeoutev = event_new(base, -1, EV_PERSIST, timeout_cb, (void* )&timeoutev);
    struct timeval tv;

    evutil_timerclear(&tv);
    tv.tv_sec = 20;
    //event_add(&timeout, &tv);
    event_add(timeoutev, &tv);
    evutil_gettimeofday(&lasttime, NULL);

    event_base_dispatch(base);

    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);
    delete (evthread);

    std::cout << "Done!" << std::endl;

    return 0;
}
