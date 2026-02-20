#pragma once

#include <qstring>

/// @return 如果Pattern不包含路径非法字符并且包含未转义的@与#返回true，否则返回false。
bool isLegalRenamePattern(const QString& renamePattern);

/// @note 断言Pattern一定是合法的。
QString parseRenamePattern(const QString& renamePattern, const QString& str, int num);
