#include "file_logger.h"

#include <qdatetime.h>
#include <qdir.h>
#include <qfileinfo.h>

#include "utils/logging.h"

FileLogger::FileLogger(QObject* parent)
    : QObject(parent)
{}

FileLogger::~FileLogger()
{
    filepath_.clear();
    stream_.setDevice(nullptr);
    if (file_.isOpen())
        file_.close();
    qInstallMessageHandler(nullptr);
    isSetup_.store(false);
}

FileLogger& FileLogger::getInstance()
{
    static FileLogger instance;
    return instance;
}

QString FileLogger::filepath() const
{
    QMutexLocker locker(&mutex_);
    return filepath_;
}

void FileLogger::setFilepath(const QString& filepath)
{
    QMutexLocker locker(&mutex_);
    filepath_ = filepath;
}

void FileLogger::install()
{
    qInstallMessageHandler(customMessageHandler);
}

void FileLogger::uninstall()
{
    qInstallMessageHandler(nullptr);

    QMutexLocker locker(&mutex_);

    filepath_.clear();
    stream_.setDevice(nullptr);
    if (file_.isOpen())
        file_.close();
    isSetup_.store(false);
}

bool FileLogger::setup()
{
    QMutexLocker locker(&mutex_);

    if (filepath_.isEmpty())
        return false;

    QDir dir = QFileInfo(filepath_).absoluteDir();
    if (!dir.exists() && !dir.mkpath("."))
    {
        debugOut(qCritical(), "[File Logger] Failed to create log directory: %1.", dir.absolutePath());
        return false;
    }

    file_.setFileName(filepath_);
    if (file_.open(QIODevice::Append | QIODevice::Text))
    {
        stream_.setDevice(&file_);
        isSetup_.store(true);
        return true;
    }
    else
    {
        debugOut(qCritical(), "[File Logger] Failed to open log file: %1.", file_.fileName());
        return false;
    }
}

bool FileLogger::isSetup() const
{
    return isSetup_.load();
}

QString FileLogger::messageTypeString(QtMsgType type)
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

void FileLogger::customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (!getInstance().isSetup())
    {
        if (!getInstance().setup())
        {
            debugOut(qWarning(), "Failed to setup File Logger.");
            return;
        }
    }

    QMutexLocker locker(&getInstance().mutex_);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logMessage = QString("%1 [%2] %3").arg(timestamp, messageTypeString(type), msg);

    getInstance().stream_ << logMessage << "\n";
    getInstance().stream_.flush();
}
