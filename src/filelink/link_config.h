#pragma once

#include <qstring>

struct LinkConfig
{
    // 如果为真，意味着当链接操作出现错误时，所有任务处理完成之后仍然显示进度对话框。
    bool keepDialogOnErrorOccurred    = false;
    // 如果为真，执行替换操作时，旧项目会被移动至回收站而不是直接删除。
    bool removeToTrash                  = false;
};
