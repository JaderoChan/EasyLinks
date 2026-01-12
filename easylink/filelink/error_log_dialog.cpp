#include "error_log_dialog.h"

#include <qdatetime.h>

#include <easy_translate.hpp>

#define CLSNAME "LogDialog"

ErrorLogDialog::ErrorLogDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

void ErrorLogDialog::appendLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg)
{
    auto log = QString(
        "<div style='margin: 1px 0; line-height: 1.2;'>"
        "<span style='color:gray;'>[%1]</span><br>"
        "Failed to %2 the<br>"
        "<a href='folder://%3' style='color:blue;text-decoration:underline;'>%3</a> "
        "<br>to<br>"
        "<a href='folder://%4' style='color:blue;text-decoration:underline;'>%4</a>"
        "<br>"
        "Error message: <span style='color:red;'>%5</span>"
        "</div><br>"
    ).arg(
        currentTimeString(),
        linkType == LT_SYMLINK ? "Symlink" : "Hardlink",
        entryPair.source.absolutePath(),
        entryPair.target.absolutePath(),
        errorMsg
    );
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
    return QString("%1-%2-%3 %4:%5:%6.%7")
        .arg(formatNumberString(dt.date().year()))
        .arg(formatNumberString(dt.date().month()))
        .arg(formatNumberString(dt.date().day()))
        .arg(formatNumberString(dt.time().hour()))
        .arg(formatNumberString(dt.time().minute()))
        .arg(formatNumberString(dt.time().second()))
        .arg(formatNumberString(dt.time().msec()));
}
