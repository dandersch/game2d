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

void eventsys_subscribe(std::function<void(const Event&)> handler);
void eventsys_send_event(const Event& evn);

// unsubscribes given function from events by finding & removing it
void eventsys_unsubscribe(std::function<void(const Event&)> handler);
