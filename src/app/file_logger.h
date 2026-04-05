#pragma once

#include <atomic>

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qobject.h>
#include <qmutex.h>

// 单例类
class FileLogger : public QObject
{
public:
    static FileLogger& getInstance();

    QString filepath() const;
    void setFilepath(const QString& filepath);

    void install();
    void uninstall();

protected:
    static QString messageTypeString(QtMsgType type);
    static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    explicit FileLogger(QObject* parent = nullptr);
    ~FileLogger();

    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

    bool setup();
    bool isSetup() const;

    std::atomic<bool> isSetup_{false};
    mutable QMutex mutex_;
    QString filepath_;
    QFile file_;
    QTextStream stream_;
};
