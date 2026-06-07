#pragma once

// 可以通过位或运算组合多个模式。
enum Pattern : int
{
    PATTERN_SAME_NAME = 0x01, ///< 具有相同的文件名，包括后缀名
    PATTERN_SAME_SIZE = 0x02, ///< 具有相同的文件大小
    PATTERN_SAME_PERM = 0x03, ///< 具有相同的权限
    PATTERN_SAME_HASH = 0x03  ///< 文件内容的 Hash 值相同
};

using Patterns = int;
