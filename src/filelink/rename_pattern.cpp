#include "rename_pattern.h"

static bool isLegalPathChar(QChar ch)
{
    return (
        ch != '/' && ch != '\\' &&
        ch != ':' && ch != '*' &&
        ch != '?' && ch != '"' &&
        ch != '<' && ch != '>' &&
        ch != '|'
    );
}

bool isLegalRenamePattern(const QString& renamePattern)
{
    bool hasAt = false;
    bool hasSharp = false;

    bool isEscape = false;
    for (QChar ch : renamePattern)
    {
        if (!isLegalPathChar(ch) && ch != '\\')
            return false;

        if (ch == '\\')
        {
            isEscape = true;
            continue;
        }
        else if (ch == '@')
        {
            if (!isEscape)
                hasAt = true;
            else
                isEscape = false;
        }
        else if (ch == '#')
        {
            if (!isEscape)
                hasSharp = true;
            else
                isEscape = false;
        }
        else
        {
            if (isEscape)
                return false;
        }

        isEscape = false;
    }

    return hasAt && hasSharp && !isEscape;
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
