#pragma once

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qobject.h>

// 单例类
class LogManager : public QObject
{
public:
    static LogManager& getInstance();

    bool setup(const QString& filepath);
    void cleanup();

protected:
    static QString messageTypeString(QtMsgType type);
    static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    explicit LogManager(QObject* parent = nullptr);
    ~LogManager();

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    QFile file_;
    QTextStream stream_;
};
