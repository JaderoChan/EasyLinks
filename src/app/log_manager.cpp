#include "log_manager.h"

#include <qdatetime.h>

LogManager::LogManager(QObject* parent)
    : QObject(parent)
{}

LogManager::~LogManager()
{
    cleanup();
}

LogManager& LogManager::getInstance()
{
    static LogManager instance;
    return instance;
}

bool LogManager::setup(const QString& filepath)
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

void LogManager::cleanup()
{
    qInstallMessageHandler(nullptr);
    if (file_.isOpen())
        file_.close();
}

QString LogManager::messageTypeString(QtMsgType type)
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

void LogManager::customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logMessage = QString("%1 [%2] %3").arg(timestamp, messageTypeString(type), msg);

    getInstance().stream_ << logMessage << "\n";
    getInstance().stream_.flush();
}
