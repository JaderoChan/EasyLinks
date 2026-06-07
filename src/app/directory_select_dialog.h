#pragma once

#include <qdialog.h>
#include <qevent.h>
#include <qstringlist.h>

#include "ui_directory_select_dialog.h"

class DirectorySelectDialog : QDialog
{
    Q_OBJECT

public:
    explicit DirectorySelectDialog(QWidget* parent = nullptr);

    void addDir(const QString& dir);
    void addDir(const QStringList& dirs);
    void removeDir(const QString& dir);
    void removeDir(const QStringList& dirs);

signals:
    void selectFinished(QStringList dirs);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void updateText();

private:
    void updateUi();

    Ui::DirectorySelectDialog ui;
    QSet<QString> dirs_;
};
