#include "file_log_manager.h"

#include <qdatetime.h>

FileLogManager::FileLogManager(QObject* parent)
    : QObject(parent)
{}

FileLogManager::~FileLogManager()
{
    cleanup();
}

FileLogManager& FileLogManager::getInstance()
{
    static FileLogManager instance;
    return instance;
}

bool FileLogManager::setup(const QString& filepath)
{
    file_.setFileName(filepath);
    if (file_.open(QIODevice::Append | QIODevice::Text))
    {
        stream_.setDevice(&file_);
        qInstallMessageHandler(customMessageHandler);
        return true;
    }
    return false;
}

void FileLogManager::cleanup()
{
    qInstallMessageHandler(nullptr);
    if (file_.isOpen())
        file_.close();
}

QString FileLogManager::messageTypeString(QtMsgType type)
{
    switch (type)
    {
        case QtDebugMsg:
            return "Debug";
        case QtInfoMsg:
            return "Info";
        case QtWarningMsg:
            return "Warning";
        case QtCriticalMsg:
            return "Critical";
        case QtFatalMsg:
            return "Fatal";
        default:
            return "Unknown";
    }
}

void FileLogManager::customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logMessage = QString("%1 [%2] %3").arg(timestamp, messageTypeString(type), msg);

    getInstance().stream_ << logMessage << "\n";
    getInstance().stream_.flush();
}
