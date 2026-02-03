#include "rename_pattern.h"

bool isLegalRenamePattern(const QString& renamePattern)
{
    bool hasAt = false;
    bool hasSharp = false;

    bool isEscape = false;
    for (QChar ch : renamePattern)
    {
        if (ch == '\\')
        {
            isEscape = true;
            continue;
        }

        if (!isEscape)
        {
            if (ch == '@')
                hasAt = true;
            else if (ch == '#')
                hasSharp = true;
        }

        isEscape = false;
    }

    return hasAt && hasSharp;
}

QString parseRenamePattern(const QString& renamePattern, const QString& str, int num)
{
    QString result;

    bool isEscape = false;
    for (QChar ch : renamePattern)
    {
        if (ch == '\\')
        {
            isEscape = true;
            continue;
        }

        if (ch == '@')
            result += isEscape ? ch : str;
        else if (ch == '#')
            result += isEscape ? ch : QString::number(num);
        else
            result += ch;

        isEscape = false;
    }

    return result;
}
