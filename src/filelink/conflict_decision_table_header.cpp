#include "conflict_decision_table_header.h"

#include <qlayout.h>

ConflictDecisionTableHeader::ConflictDecisionTableHeader(QWidget* parent)
    : QWidget(parent), label_(this), cb_(this)
{
    setupUi();
}

ConflictDecisionTableHeader::ConflictDecisionTableHeader(const QString& text, QWidget* parent)
    : ConflictDecisionTableHeader(parent)
{
    label_.setText(text);
}

int ConflictDecisionTableHeader::spacing() const
{
    return layout()->spacing();
}

void ConflictDecisionTableHeader::setSpacing(int px)
{
    layout()->setSpacing(px);
}

bool ConflictDecisionTableHeader::eventFilter(QObject* obj, QEvent* event)
{
    // 拦截CheckBox的鼠标按钮事件。
    bool isMouseButtonEvent =
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseButtonRelease;
    return isMouseButtonEvent;
}

bool ConflictDecisionTableHeader::event(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::MouseButtonPress:  // Fallthrough
        case QEvent::MouseButtonDblClick:
        {
            auto* e = dynamic_cast<QMouseEvent*>(event);
            if (e)
            {
                if (e->button() == Qt::LeftButton && rect().contains(e->pos()))
                    pressed_ = true;
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            auto* e = dynamic_cast<QMouseEvent*>(event);
            if (e)
            {
                if (e->button() == Qt::LeftButton && rect().contains(e->pos()))
                    emit clicked();
                pressed_ = false;
                return true;
            }
            break;
        }
        default:
            break;
    }

    return QWidget::event(event);
}

void ConflictDecisionTableHeader::setupUi()
{
    cb_.setFocusPolicy(Qt::NoFocus);
    cb_.setAttribute(Qt::WA_TransparentForMouseEvents);
    cb_.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cb_.installEventFilter(this);

    label_.setOpenExternalLinks(true);
    label_.setAlignment(Qt::AlignLeft);
    label_.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&cb_);
    layout->addWidget(&label_);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}
