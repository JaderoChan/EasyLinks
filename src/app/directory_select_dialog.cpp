#include "directory_select_dialog.h"

#include <qfiledialog.h>
#include <qmimedata.h>

#include <easy_translate.hpp>

DirectorySelectDialog::DirectorySelectDialog(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.listWidget->installEventFilter(this);

    // Add button
    connect(ui.addButton, &QPushButton::clicked, this, [this]()
    {
        QString dir = QFileDialog::getExistingDirectory(
            this, EASYTR("Add directory"), QDir::homePath());
        addDir(dir);
    });

    // Remove button
    connect(ui.removeButton, &QPushButton::clicked, this, [this]()
    {
        const auto& list = ui.listWidget->selectedItems();
        if (!list.empty())
        {
            for (auto* item : list)
            {
                if (item)
                {
                    dirs_.remove(item->text());
                    delete item;
                }
            }
        }

        updateUi();
    });

    // Start/Finish button
    connect(ui.startButton, &QPushButton::clicked, this, [this]()
    {
        QStringList dirs;
        for (const auto& dir : dirs_)
            dirs.append(dir);
        emit selectFinished(dirs);
    });

    updateUi();
    updateText();
}

void DirectorySelectDialog::addDir(const QString& dir)
{
    if (!dir.isEmpty() && !dirs_.contains(dir))
    {
        dirs_.insert(dir);
        QListWidgetItem* item = new QListWidgetItem(dir, ui.listWidget);
        item->setToolTip(dir);
        updateUi();
    }
}

void DirectorySelectDialog::addDir(const QStringList& dirs)
{
    if (dirs.empty())
        return;

    for (const auto& dir : dirs)
    {
        if (!dir.isEmpty() && !dirs_.contains(dir))
        {
            dirs_.insert(dir);
            QListWidgetItem* item = new QListWidgetItem(dir, ui.listWidget);
            item->setToolTip(dir);
        }
    }

    updateUi();
}

void DirectorySelectDialog::removeDir(const QString& dir)
{
    if (dirs_.contains(dir))
    {
        dirs_.remove(dir);
        for (size_t i = 0; i < ui.listWidget->count(); ++i)
        {
            if (ui.listWidget->item(static_cast<int>(i))->text() == dir)
            {
                auto* item = ui.listWidget->takeItem(static_cast<int>(i));
                delete item;
                break;
            }
        }
        updateUi();
    }
}

void DirectorySelectDialog::removeDir(const QStringList& dirs)
{
    QSet<QString> dirSet(dirs.begin(), dirs.end());

    QList<QListWidgetItem*> items;
    for (size_t i = 0; i < ui.listWidget->count(); ++i)
    {
        const auto dir = ui.listWidget->item(static_cast<int>(i))->text();
        if (dirSet.contains(dir))
            items.append(ui.listWidget->takeItem(i));
    }

    for (auto* item : items)
    {
        if (item)
        {
            dirs_.remove(item->text());
            delete item;
        }
    }

    updateUi();
}

static bool allUrlsIsDir(const QMimeData* md)
{
    if (!md || !md->hasUrls())
        return false;

    const auto& urls = md->urls();
    if (urls.empty())
        return false;

    for (const auto& url : urls)
    {
        if (!url.isLocalFile())
            return false;

        QFileInfo fi(url.toLocalFile());
        if (!fi.exists() || !fi.isDir())
            return false;
    }

    return true;
}

bool DirectorySelectDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui.listWidget)
    {
        switch (event->type())
        {
            case QEvent::DragEnter:
            {
                auto* e = static_cast<QDragEnterEvent*>(event);
                if (allUrlsIsDir(e->mimeData()))
                    e->acceptProposedAction();
                return true;
            }
            case QEvent::DragMove:
            {
                auto* e = static_cast<QDragMoveEvent*>(event);
                if (allUrlsIsDir(e->mimeData()))
                    e->acceptProposedAction();
                return true;
            }
            case QEvent::Drop:
            {
                auto* e = static_cast<QDropEvent*>(event);
                if (allUrlsIsDir(e->mimeData()))
                {
                    QStringList dirs;
                    for (const auto& url : e->mimeData()->urls())
                        dirs.append(url.toLocalFile());
                    addDir(dirs);
                    e->acceptProposedAction();
                }
                return true;
            }
            default:
                break;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void DirectorySelectDialog::updateText()
{
    setWindowTitle(EASYTR("DirectorySelectDialog.Title"));
    ui.tipTextLabel->setText(QString(EASYTR("DirectorySelectDialog.TipText.Text")).arg(dirs_.count()));
    ui.tipIconLabel->setToolTip(EASYTR("DirectorySelectDialog.TipIcon.ToolTip"));
    ui.addButton->setText(EASYTR("DirectorySelectDialog.AddButton.Text"));
    ui.removeButton->setText(EASYTR("DirectorySelectDialog.RemoveButton.Text"));
    ui.startButton->setText(EASYTR("DirectorySelectDialog.StartButton.Text"));
}

void DirectorySelectDialog::updateUi()
{
    ui.tipTextLabel->setText(QString(EASYTR("DirectorySelectDialog.TipText.Text")).arg(dirs_.count()));
    ui.removeButton->setEnabled(!ui.listWidget->selectedItems().isEmpty());
    ui.startButton->setEnabled(!dirs_.isEmpty());
}
