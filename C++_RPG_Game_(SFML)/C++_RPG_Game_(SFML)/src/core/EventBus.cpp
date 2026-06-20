#include "EventBus.h"

namespace RPG {
    EventBus& EventBus::getInstance() {
        static EventBus instance;
        return instance;
    }

    void EventBus::subscribe(EventType type, EventCallback callback) {
        m_subscribers[type].push_back(std::move(callback));
    }

    void EventBus::publish(const GameEvent& event) {
        if (m_subscribers.find(event.getType()) != m_subscribers.end()) {
            for (auto& cb : m_subscribers[event.getType()]) {
                cb(event);
            }
        }
    }
}
