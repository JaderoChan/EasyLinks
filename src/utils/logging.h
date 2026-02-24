#pragma once

#include <qdebug.h>
#include <qstring.h>

template <typename DebugIO>
void qlog(DebugIO io, const QString& formatStr)
{
    io.noquote() << QString(formatStr);
}

template <typename DebugIO, typename ...Args>
void qlog(DebugIO io, const QString& formatStr, Args... args)
{
    io.noquote() << QString(formatStr).arg(args...);
}
