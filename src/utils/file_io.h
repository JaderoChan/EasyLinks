#pragma once

#include <qfile.h>
#include <qstring.h>

/// @throw std::runtime_error
QString readAllFromFile(const QString& filePath);
