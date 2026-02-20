#pragma once

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qobject.h>
#include <qmutex.h>

// 单例类
class FileLogManager : public QObject
{
public:
    static FileLogManager& getInstance();

    bool setup(const QString& filepath);
    void cleanup();

protected:
    static QString messageTypeString(QtMsgType type);
    static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    explicit FileLogManager(QObject* parent = nullptr);
    ~FileLogManager();

    FileLogManager(const FileLogManager&) = delete;
    FileLogManager& operator=(const FileLogManager&) = delete;

    QMutex mutex_;
    QFile file_;
    QTextStream stream_;
};
