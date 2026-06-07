#pragma once

#include <stdexcept>
#include <type_traits>

#include <qbytearray.h>
#include <qfile.h>
#include <qstring.h>

/// @brief 读取整个文件的内容。
/// @tparam asText 为 `true` 时返回 `QString`，为 `false` 时返回 `QByteArray`。
/// @throw std::runtime_error
template <bool asText = true, typename T = std::conditional_t<asText, QString, QByteArray>>
inline T readFileContent(const QString& filePath)
{
    QFile file(filePath);
    QIODevice::OpenMode mode = asText ? (QIODevice::ReadOnly | QIODevice::Text) : QIODevice::ReadOnly;
    if (!file.open(mode))
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());

    if constexpr (asText)   return QString::fromUtf8(file.readAll());
    else                    return file.readAll();
}

/// @brief 将内容写入文件。
/// @tparam asText 为 `true` 时写入 `QString`为文本文件，为 `false` 时写入 `QByteArray` 为二进制文件。
/// @throw std::runtime_error
template <bool asText = true, typename T = std::conditional_t<asText, QString, QByteArray>>
inline void writeFileContent(const QString& filePath, const T& content, bool isAppend = false)
{
    QFile file(filePath);
    QIODevice::OpenMode mode = asText ? (QIODevice::WriteOnly | QIODevice::Text) : QIODevice::WriteOnly;
    mode = isAppend ? (mode | QIODevice::Append) : mode;
    if (!file.open(mode))
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());

    if constexpr (asText)   file.write(content.toUtf8());
    else                    file.write(content);
}
