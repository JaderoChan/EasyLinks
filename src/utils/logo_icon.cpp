#include "logo_icon.h"

static QIcon createLogoIcon()
{
    QIcon icon;
    icon.addFile(":/icons/app_16.png");
    icon.addFile(":/icons/app_32.png");
    icon.addFile(":/icons/app_64.png");
    icon.addFile(":/icons/app_128.png");
    icon.addFile(":/icons/app_256.png");
    icon.addFile(":/icons/app_512.png");
    return icon;
}

QIcon getLogoIcon()
{
    static QIcon icon = createLogoIcon();
    return icon;
}

QPixmap getLogoPixmap()
{
    static QPixmap pixmap(":/icons/app_512.png");
    return pixmap;
}
