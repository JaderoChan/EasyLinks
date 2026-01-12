#pragma once

#include <qdialog.h>
#include <qevent.h>
#include <qstring>

#include "ui_error_log_dialog.h"
#include "types.h"

class ErrorLogDialog : public QDialog
{
public:
    explicit ErrorLogDialog(QWidget* parent = nullptr);

    void appendLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg);

protected:
    virtual void updateText();

    void changeEvent(QEvent* event) override;

private:
    static QString currentTimeString();

    Ui::ErrorLogDialog ui;
};
