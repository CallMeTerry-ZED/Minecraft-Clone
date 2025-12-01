/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef EVENT_H
#define EVENT_H

#pragma once

#include <functional>
#include <vector>
#include <memory>

namespace MinecraftClone
{
    enum class EventType
    {
        None = 0,
        WindowClose,
        WindowResize,
        KeyPressed,
        KeyReleased,
        KeyRepeat,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryWindow = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3
    };

    class Event
    {
    public:
        virtual ~Event() = default;
        virtual EventType GetEventType() const = 0;
        virtual int GetCategoryFlags() const = 0;
        bool IsInCategory(EventCategory category) const
        {
            return GetCategoryFlags() & category;
        }

        bool m_handled = false;
    };

    class WindowCloseEvent : public Event
    {
    public:
        EventType GetEventType() const override { return EventType::WindowClose; }
        int GetCategoryFlags() const override { return EventCategoryWindow; }
    };

    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int width, int height) : m_width(width), m_height(height) {}

        EventType GetEventType() const override { return EventType::WindowResize; }
        int GetCategoryFlags() const override { return EventCategoryWindow; }

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        int m_width;
        int m_height;
    };

    class KeyEvent : public Event
    {
    public:
        KeyEvent(int key, int scancode, int mods)
            : m_key(key), m_scancode(scancode), m_mods(mods) {}

        int GetKey() const { return m_key; }
        int GetScancode() const { return m_scancode; }
        int GetMods() const { return m_mods; }

        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryKeyboard; }

    private:
        int m_key;
        int m_scancode;
        int m_mods;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int key, int scancode, int mods, bool repeat)
            : KeyEvent(key, scancode, mods), m_repeat(repeat) {}

        EventType GetEventType() const override { return EventType::KeyPressed; }
        bool IsRepeat() const { return m_repeat; }

    private:
        bool m_repeat;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(int key, int scancode, int mods)
            : KeyEvent(key, scancode, mods) {}

        EventType GetEventType() const override { return EventType::KeyReleased; }
    };

    class MouseButtonEvent : public Event
    {
    public:
        MouseButtonEvent(int button, int mods) : m_button(button), m_mods(mods) {}

        int GetButton() const { return m_button; }
        int GetMods() const { return m_mods; }

        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

    private:
        int m_button;
        int m_mods;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(int button, int mods) : MouseButtonEvent(button, mods) {}

        EventType GetEventType() const override { return EventType::MouseButtonPressed; }
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(int button, int mods) : MouseButtonEvent(button, mods) {}

        EventType GetEventType() const override { return EventType::MouseButtonReleased; }
    };

    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(float x, float y) : m_x(x), m_y(y) {}

        EventType GetEventType() const override { return EventType::MouseMoved; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

        float GetX() const { return m_x; }
        float GetY() const { return m_y; }

    private:
        float m_x;
        float m_y;
    };

    class MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {}

        EventType GetEventType() const override { return EventType::MouseScrolled; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

        float GetXOffset() const { return m_xOffset; }
        float GetYOffset() const { return m_yOffset; }

    private:
        float m_xOffset;
        float m_yOffset;
    };

    using EventCallback = std::function<void(Event&)>;
}

#endif