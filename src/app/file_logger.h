#pragma once

#include <qfile.h>
#include <qfilesystemwatcher.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qobject.h>
#include <qmutex.h>

// 单例类
class FileLogger : public QObject
{
public:
    static FileLogger& getInstance();

    bool setup(const QString& filepath);
    void cleanup();

protected:
    static QString messageTypeString(QtMsgType type);
    static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    explicit FileLogger(QObject* parent = nullptr);
    ~FileLogger();

    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

    bool setFileAndStream();

    QString filePath_;
    QMutex mutex_;
    QFile file_;
    QTextStream stream_;
    QFileSystemWatcher fileWatcher_;
};
