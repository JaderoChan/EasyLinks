#include "qwidget_utils.h"

void showAndActivate(QWidget* widget)
{
    widget->show();
    widget->raise();
    widget->activateWindow();
}
