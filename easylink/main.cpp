#include <qapplication.h>

#include "filelink/filelink_manager.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    // a.setQuitOnLastWindowClosed(false);

    QString source("E:/05_Data");
    QString target("E:/06_TMP/Test");
    FileLinkManager m;
    m.createLinks(LT_HARDLINK, {source}, target);

    int ret = a.exec();

    return ret;
}
