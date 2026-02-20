#pragma once

#include <qstring.h>

/// @brief 保证数字具有两位，使用0进行左补位。
inline QString getPreferredNumberString(int num)
{
    return QString("%1").arg(num, 2, 10, QChar('0'));
}
