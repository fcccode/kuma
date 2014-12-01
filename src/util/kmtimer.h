/* Copyright (c) 2014, Fengping Bao <jamol@live.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __KMTIMER_H__
#define __KMTIMER_H__

#include "kmmutex.h"
#include <map>

#define TICK_COUNT_TYPE	unsigned long

namespace komm {;

#define TIMER_VECTOR_BITS   8
#define TIMER_VECTOR_SIZE   (1 << TIMER_VECTOR_BITS)
#define TIMER_VECTOR_MASK   (TIMER_VECTOR_SIZE - 1)
#define TV_COUNT            4

class TimerHandler
{
public:
    virtual ~TimerHandler() {}

    virtual int onTimer() = 0;
};

class KM_Timer;
class KM_Timer_Manager;

class KM_Timer_Node
{
public:
    KM_Timer_Node()
    {
        cancelled_ = false;
        elapse_ = 0;
        start_tick_ = 0;
        timer_ = NULL;
        reset();
    }
    void reset()
    {
        prev = NULL;
        next = NULL;
    }

    bool            cancelled_;
    unsigned int	elapse_;
    TICK_COUNT_TYPE start_tick_;
    KM_Timer*	    timer_;

protected:
    friend class KM_Timer_Manager;
    KM_Timer_Node* prev;
    KM_Timer_Node* next;
};

class KM_Timer
{
public:
    KM_Timer(KM_Timer_Manager* mgr, TimerHandler* hdr);
    ~KM_Timer();

    void on_timer();
    bool schedule(unsigned int time_elapse);
    void schedule_cancel();

    void on_detach()
    {
        m_timer_mgr = NULL; // may thread conflict
    }

private:
    KM_Timer_Manager*	m_timer_mgr;
    KM_Timer_Node       m_timer_node;
    TimerHandler*       m_handler;
};

class KM_Timer_Manager
{
public:
    KM_Timer_Manager();
    ~KM_Timer_Manager();

    bool schedule(KM_Timer_Node* timer_node, unsigned int time_elapse);
    void schedule_cancel(KM_Timer_Node* timer_node);

    int check_expire(unsigned long& remain_time);

private:
    bool add_timer(KM_Timer_Node* timer_node, bool from_schedule=false);
    void remove_timer(KM_Timer_Node* timer_node);
    int cascade_timer(int tv_idx, int tl_idx);
    bool timer_pending(KM_Timer_Node* timer_node)
    {
        return timer_node->next != NULL;
    }

    void list_init_head(KM_Timer_Node* head);
    void list_add_node(KM_Timer_Node* head, KM_Timer_Node* timer_node);
    void list_remove_node(KM_Timer_Node* timer_node);
    void list_replace(KM_Timer_Node* old_head, KM_Timer_Node* new_head);
    void list_combine(KM_Timer_Node* from_head, KM_Timer_Node* to_head);
    bool list_empty(KM_Timer_Node* head);

private:
    KM_Mutex m_mutex;
    KM_Mutex m_running_mutex;
    KM_Timer_Node*  m_running_node;
    TICK_COUNT_TYPE m_last_tick;
    unsigned int m_timer_count;
    KM_Timer_Node m_tv[TV_COUNT][TIMER_VECTOR_SIZE]; // timer vectors
};

}

#endif
