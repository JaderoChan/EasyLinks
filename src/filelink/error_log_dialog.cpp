#include "error_log_dialog.h"

#include <qdatetime.h>

#include <easy_translate.hpp>

#define CLSNAME "ErrorLogDialog"
#define ERROR_LOG_FORMAT_STRING \
    "<div style='margin: 1px 0; line-height: 1.2;'>" \
    "<span style='color:gray;'>[%1]</span><br>" \
    "Failed to %2 the<br>" \
    "<a href='%3' style='color:rgba(0, 100, 180, 216);text-decoration:none;'>%4</a> " \
    "<br>to<br>" \
    "<a href='%5' style='color:rgba(0, 100, 180, 216);text-decoration:none;'>%6</a>" \
    "<br>" \
    "Error message: <span style='color:red;'>%7</span>" \
    "</div><br>"

ErrorLogDialog::ErrorLogDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    updateText();
}

void ErrorLogDialog::appendLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg)
{
    auto log = QString(ERROR_LOG_FORMAT_STRING).arg(
        currentTimeString(),
        linkType == LT_SYMLINK ? "Symlink" : "Hardlink",
        entryPair.source.fileinfo.absolutePath(),
        entryPair.source.fileinfo.absoluteFilePath(),
        entryPair.target.fileinfo.absolutePath(),
        entryPair.target.fileinfo.absoluteFilePath(),
        errorMsg);
    ui.log->append(log);
}

void ErrorLogDialog::updateText()
{
    setWindowTitle(EASYTR(CLSNAME ".WindowTitle"));
}

void ErrorLogDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QDialog::changeEvent(event);
}

QString ErrorLogDialog::currentTimeString()
{
    static auto formatNumberString = [](int num) -> QString
    { return (num < 10 ? "0" : "") + QString::number(num); };

    auto dt = QDateTime::currentDateTime();
    return QString("%1-%2-%3 %4:%5:%6.%7").arg(
        formatNumberString(dt.date().year()),
        formatNumberString(dt.date().month()),
        formatNumberString(dt.date().day()),
        formatNumberString(dt.time().hour()),
        formatNumberString(dt.time().minute()),
        formatNumberString(dt.time().second()),
        formatNumberString(dt.time().msec())
    );
}
