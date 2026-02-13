#include "keycombination_inputer.h"

KeyCombinationInputer::KeyCombinationInputer(QWidget* parent)
    : QLineEdit(parent)
{
    // 只接受点击获取焦点。
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    // 禁用右键上下文菜单。
    setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    // 居中文本。
    setAlignment(Qt::AlignmentFlag::AlignCenter);
    // 禁止编辑。
    setReadOnly(true);
    // 设置光标图形为#PointingHandCursor。
    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    setPlaceholderText(noneKcText_);
}

QKeyCombination KeyCombinationInputer::keyCombination() const
{
    return kc_;
}

void KeyCombinationInputer::setWaitingText(const QString& text)
{
    waitingText_ = text;
    updateText();
}

void KeyCombinationInputer::setNoneKeyCombinationText(const QString& text)
{
    setPlaceholderText(noneKcText_);
}

void KeyCombinationInputer::setKeyCombination(const QKeyCombination& keyCombination)
{
    auto newKc = keyCombination;
    if (newKc != kc_)
    {
        kc_ = newKc;
        emit keyCombinationChanged(kc_);
    }
    updateText();
}

void KeyCombinationInputer::setKeyCombination(const QKeySequence& keySequence)
{
    auto newKc = keySequence.isEmpty() ? QKeyCombination() : keySequence[0];
    if (newKc != kc_)
    {
        kc_ = newKc;
        emit keyCombinationChanged(kc_);
    }
    updateText();
}

bool KeyCombinationInputer::isVaild(int key, Qt::KeyboardModifiers mod)
{
    bool keyIsValid =
        (key >= Qt::Key::Key_A && key <= Qt::Key::Key_Z) ||
        (key >= Qt::Key::Key_F1 && key <= Qt::Key::Key_F24) ||
        (key >= Qt::Key::Key_Space && key <= Qt::Key::Key_Slash) ||
        (key >= Qt::Key::Key_Colon && key <= Qt::Key::Key_At) ||
        (key >= Qt::Key::Key_BracketLeft && key <= Qt::Key::Key_QuoteLeft) ||
        (key >= Qt::Key::Key_BraceLeft && key <= Qt::Key::Key_AsciiTilde) ||
        (key >= Qt::Key::Key_Tab && key <= Qt::Key::Key_PageDown);

    bool modIsValid =
        (mod & Qt::Modifier::CTRL) ||
        (mod & Qt::Modifier::ALT) ||
        (mod & Qt::Modifier::SHIFT);

    return keyIsValid && modIsValid;
}

void KeyCombinationInputer::focusInEvent(QFocusEvent* event)
{
    QLineEdit::focusInEvent(event);
    if (event->type() == QEvent::Type::FocusIn)
    {
        isWaitingInput_ = true;
        updateText();
    }
}

void KeyCombinationInputer::focusOutEvent(QFocusEvent* event)
{
    QLineEdit::focusOutEvent(event);
    if (isWaitingInput_)
    {
        isWaitingInput_ = false;
        updateText();
    }
}

void KeyCombinationInputer::keyPressEvent(QKeyEvent* event)
{
    if (isWaitingInput_)
    {
        auto key = event->key();
        auto mod = event->modifiers();

        // 如果输入了ESC键，则恢复至原状态。
        if (key == Qt::Key::Key_Escape)
        {
            isWaitingInput_ = false;
            updateText();
            clearFocus();
        }
        // 如果输入了Delete键，则删除组合键。
        else if (key == Qt::Key::Key_Delete && mod == 0)
        {
            isWaitingInput_ = false;
            setKeyCombination(QKeySequence());
            clearFocus();
        }
        // 等待一个有效输入。
        else if (isVaild(key, mod))
        {
            isWaitingInput_ = false;
            setKeyCombination(QKeyCombination(mod, static_cast<Qt::Key>(key)));
            clearFocus();
        }
    }
}

void KeyCombinationInputer::mouseMoveEvent(QMouseEvent* event)
{
    event->ignore();
}

void KeyCombinationInputer::mouseDoubleClickEvent(QMouseEvent* event)
{
    event->ignore();
}

void KeyCombinationInputer::updateText()
{
    isWaitingInput_ ? setText(waitingText_) : setText(QKeySequence(kc_).toString(QKeySequence::NativeText));
}
