#pragma once

#include "pch.h"

// TODO large enough?
#define MAX_EVENT_SUBS 500

enum class EventType
{
    EVENT_REWIND,
    EVENT_RESTART,
    EVENT_COUNT
};

struct Event
{
    EventType type;
    mutable bool handled;
    std::unordered_map<std::string, void*> args; // for custom properties

    Event(EventType type) : type(type), handled(false)
    {
        args = std::unordered_map<std::string, void*>();
    };
};

class EventSystem
{
public:
    static void subscribe(std::function<void(const Event&)> handler)
    {
        onEvent[sub_count++] = handler;
    }

    static void sendEvent(const Event& evn)
    {
        for (u32 i = 0; i < sub_count; i++)
        {
            if (evn.handled) return;
            onEvent[i](evn);
        }
    }

    // unsubscribes given function from events by finding & removing it
    static void unsubscribe(std::function<void(const Event&)> handler)
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

private:
    static std::function<void(const Event&)> onEvent[MAX_EVENT_SUBS];
    static u32 sub_count;
};
