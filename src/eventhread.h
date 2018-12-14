#ifndef __EVNETHREAD_H__
#define __EVNETHREAD_H__
#include <thread>
#include <event2/util.h>
#include "itemqueue.h"
#include <memory>

class Eventthread
{
  public:
    Eventthread(std::shared_ptr<ItemQueue> queue);
    ~Eventthread();

    bool init();
    int addEvent(evutil_socket_t fd);

    void run();

    int getReadfd()
    {
        return notify_receive_fd;
    }

    int getWritefd()
    {
        return notify_send_fd;
    }

    static void
    conn_eventcb(struct bufferevent *bev, short events, void *user_data);

    static void conn_writecb(struct bufferevent *bev, void *user_data);

    static void conn_readcb(struct bufferevent *bev, void *user_data);

    static void process_new_conn(evutil_socket_t fd, short nevent, void *arg);

  private:
    std::thread thread_;
    struct event_base *base;
    int notify_receive_fd; /* receiving end of notify pipe */
    int notify_send_fd;    /* sending end of notify pipe */

    int fds[2];

    std::shared_ptr<ItemQueue> itemQueue_;
};

#endif // !__EVNETHREAD_H__