#include "file_io.h"

#include <stdexcept>

QString readAllFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Failed to open file: " + filePath.toStdString());
    return QString::fromUtf8(file.readAll());
}
