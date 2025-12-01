/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#pragma once

#include "Core/Event.h"
#include <functional>
#include <vector>
#include <memory>

namespace MinecraftClone
{
    class EventDispatcher
    {
    public:
        void Dispatch(Event& event)
        {
            for (auto& callback : m_callbacks)
            {
                if (event.m_handled)
                    break;
                callback(event);
            }
        }

        void Subscribe(EventCallback callback)
        {
            m_callbacks.push_back(callback);
        }

        void Clear()
        {
            m_callbacks.clear();
        }

    private:
        std::vector<EventCallback> m_callbacks;
    };
}

#endif