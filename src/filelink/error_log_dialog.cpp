#include "error_log_dialog.h"

#include <qdatetime.h>

#include <easy_translate.hpp>

#include "config.h"

#define CLSNAME "ErrorLogDialog"
#define ERROR_LOG_FORMAT_STRING \
"<a style='color:gray;'>[%1]</a>" \
" Failed to %2 the " \
"<a href='file:///%3' style='color:rgba(0, 100, 180, 216);text-decoration:none;'>%4</a> " \
" to " \
"<a href='file:///%5' style='color:rgba(0, 100, 180, 216);text-decoration:none;'>%6</a>" \
". Error message: <a style='color:red;'>%7</a><br>" \

ErrorLogDialog::ErrorLogDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    updateText();
}

void ErrorLogDialog::appendLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg)
{
    if (logCount_ > MAX_LOG_COUNT)
        return;

    auto log = QString(ERROR_LOG_FORMAT_STRING).arg(
        currentTimeString(),
        linkType == LT_SYMLINK ? "Symlink" : "Hardlink",
        entryPair.source.fileinfo.absolutePath(),
        entryPair.source.fileinfo.absoluteFilePath(),
        entryPair.target.fileinfo.absolutePath(),
        entryPair.target.fileinfo.absoluteFilePath(),
        errorMsg);
    ui.log->append(log);
    logCount_++;
}

void ErrorLogDialog::updateText()
{
    setWindowTitle(EASYTR(CLSNAME ".WindowTitle"));
    ui.headerText->setText(QString(EASYTR(CLSNAME ".HeaderText")).arg(MAX_LOG_COUNT));
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
