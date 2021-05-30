#include "event.h"

static std::function<void(const Event&)> onEvent[MAX_EVENT_SUBS];
static u32 sub_count = 0;

void eventsys_subscribe(std::function<void(const Event&)> handler)
{
    onEvent[sub_count++] = handler;
}

void eventsys_send_event(const Event& evn)
{
    for (u32 i = 0; i < sub_count; i++)
    {
        if (evn.handled) return;
        onEvent[i](evn);
    }
}

// unsubscribes given function from events by finding & removing it
void eventsys_unsubscribe(std::function<void(const Event&)> handler)
{
    for (u32 i = 0; i < sub_count; i++)
    {
        if (onEvent[i].target<void(*)(const Event&)>() ==
            handler.target<void(*)(const Event&)>())
        {
            if (sub_count == 1) {
                onEvent[i] = nullptr;
            } else {
                onEvent[i] = onEvent[sub_count - 1];
            }
            --sub_count;
        }
    }
}
