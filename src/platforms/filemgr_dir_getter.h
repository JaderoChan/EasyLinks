#pragma once

#include <qstring>

/**
 * @brief 获取当前处于焦点状态的文件管理器所在目录路径。
 * @return 一个目录路径。
 * @note 如果当前聚焦窗口非文件管理器（对于Windows而言是Explorer，对于MacOS而言是Finder），
 * 文件管理器所在目录不可用或路径获取失败将抛出异常。
 */
QString getFocusedFileManagerDir();
