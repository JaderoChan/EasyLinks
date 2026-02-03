#pragma once

#include <qstring>

bool isLegalRenamePattern(const QString& renamePattern);

QString parseRenamePattern(const QString& renamePattern, const QString& str, int num);
