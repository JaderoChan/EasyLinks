#pragma once

#include <qdebug.h>
#include <qstring.h>

template <typename IOType>
void qout(IOType io, const QString& formatStr)
{
    io.noquote() << QString(formatStr);
}

inline qout()

template <typename IOType, typename ...Args>
void qout(IOType io, const QString& formatStr, Args... args)
{
    io.noquote() << QString(formatStr).arg(args...);
}
