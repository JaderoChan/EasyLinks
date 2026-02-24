#pragma once

#include <qdebug.h>
#include <qstring.h>

template <typename IOType, typename ...Args>
void qout(IOType io, const char *formatStr, Args &&...args)
{
    QString str(formatStr);
    ((str = str.arg(std::forward<Args>(args))), ...);
    io.noquote() << str;
}
