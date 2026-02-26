#include "file_logger.h"

#include <qdatetime.h>
#include <qdir.h>
#include <qfileinfo.h>

#include "utils/logging.h"

FileLogger::FileLogger(QObject* parent)
    : QObject(parent)
{
    connect(&fileWatcher_, &QFileSystemWatcher::fileChanged, this, [=](const QString& path)
    {
        if (path == filePath_)
            setFileAndStream();
    });
}

FileLogger::~FileLogger()
{
    cleanup();
}

bool FileLogger::setFileAndStream()
{
    if (filePath_.isEmpty())
        return false;

    QDir dir = QFileInfo(filePath_).absoluteDir();
    if (!dir.exists() && !dir.mkpath("."))
    {
        debugOut(qCritical(), "[File Logger] Failed to create log directory: %1.", dir.absolutePath());
        return false;
    }

    file_.setFileName(filePath_);
    if (file_.open(QIODevice::Append | QIODevice::Text))
    {
        stream_.setDevice(&file_);
        return true;
    }
    else
    {
        debugOut(qCritical(), "[File Logger] Failed to open log file: %1.", file_.fileName());
        return false;
    }
}

FileLogger& FileLogger::getInstance()
{
    static FileLogger instance;
    return instance;
}

bool FileLogger::setup(const QString& filepath)
{
    if (filepath.isEmpty())
        return false;

    QMutexLocker locker(&getInstance().mutex_);

    filePath_ = filepath;
    fileWatcher_.addPath(filePath_);
    if (setFileAndStream())
    {
        qInstallMessageHandler(customMessageHandler);
        return true;
    }
    return false;
}

void FileLogger::cleanup()
{
    QMutexLocker locker(&getInstance().mutex_);

    fileWatcher_.removePath(filePath_);
    filePath_.clear();
    stream_.setDevice(nullptr);
    qInstallMessageHandler(nullptr);
    if (file_.isOpen())
        file_.close();
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
    QMutexLocker locker(&getInstance().mutex_);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logMessage = QString("%1 [%2] %3").arg(timestamp, messageTypeString(type), msg);

    getInstance().stream_ << logMessage << "\n";
    getInstance().stream_.flush();
}
