#include "event.h"

std::function<void(const Event&)> EventSystem::onEvent[MAX_EVENT_SUBS];
u32 EventSystem::sub_count = 0;
