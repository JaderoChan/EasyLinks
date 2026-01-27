#pragma once

#include <qobject.h>

#include <global_hotkey/global_hotkey.hpp>

class HotkeyHandler : public QObject
{
public:
    explicit HotkeyHandler(QObject* parent = nullptr);
    ~HotkeyHandler();

private:

};
