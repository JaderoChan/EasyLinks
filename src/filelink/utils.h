#pragma once

#include <qfileinfo.h>

#include "types.h"

// 判断所给fileinfo引用的文件实体是否为Shortcut、Junction。
bool isWindowsSymlink(const QFileInfo& fileinfo);

// - 如果目标路径不存在，会尝试创建其所有父路径。
// - 断言目标文件一定不存在。
// - 对于符号链接，会根据source引用的文件实体是file还是directory创建相应的符号链接。
// - 对于硬链接，断言source引用的文件实体一定是文件/符号链接。
void createLink(LinkType linkType, const QFileInfo& source, const QFileInfo& target);
