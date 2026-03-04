// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanvas2dutils_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qminmax.h>
#include <QtCore/private/qlocale_tools_p.h>
#include <QtCore/qstringview.h>
#include <QtGui/qfont.h>
#include <QtGui/qcolor.h>
#include <QtGui/qfontdatabase.h>

QT_BEGIN_NAMESPACE

enum FontToken
{
    NoTokens = 0x00,
    FontStyle = 0x01,
    FontVariant = 0x02,
    FontWeight = 0x04
};

QColor QCanvas2DUtils::qColorFromString(const QV4::Value &name)
{
    QByteArray str = name.toQString().toUtf8();

    char *p = str.data();
    int len = str.size();
    //rgb/hsl color string has at least 7 characters
    if (!p || len > 255 || len <= 7)
        return QColor::fromString(p);
    else {
        bool isRgb(false), isHsl(false), hasAlpha(false);
        Q_UNUSED(isHsl);

        while (isspace(*p)) p++;
        if (strncmp(p, "rgb", 3) == 0)
            isRgb = true;
        else if (strncmp(p, "hsl", 3) == 0)
            isHsl = true;
        else
            return QColor::fromString(p);

        p+=3; //skip "rgb" or "hsl"
        hasAlpha = (*p == 'a') ? true : false;

        ++p; //skip "("

        if (hasAlpha) ++p; //skip "a"

        int rh, gs, bl, alpha = 255;

        //red
        while (isspace(*p)) p++;
        rh = strtol(p, &p, 10);
        if (*p == '%') {
            rh = qRound(rh/100.0 * 255);
            ++p;
        }
        if (*p++ != ',') return QColor();

        //green
        while (isspace(*p)) p++;
        gs = strtol(p, &p, 10);
        if (*p == '%') {
            gs = qRound(gs/100.0 * 255);
            ++p;
        }
        if (*p++ != ',') return QColor();

        //blue
        while (isspace(*p)) p++;
        bl = strtol(p, &p, 10);
        if (*p == '%') {
            bl = qRound(bl/100.0 * 255);
            ++p;
        }

        if (hasAlpha) {
            if (*p++!= ',') return QColor();
            while (isspace(*p)) p++;
            bool ok = false;
            alpha = qRound(qstrtod(p, const_cast<const char **>(&p), &ok) * 255);
        }

        if (*p != ')') return QColor();
        if (isRgb)
            return QColor::fromRgba(qRgba(qBound(0, rh, 255), qBound(0, gs, 255), qBound(0, bl, 255), qBound(0, alpha, 255)));
        else if (isHsl)
            return QColor::fromHsl(qBound(0, rh, 359), qBound(0, gs, 255), qBound(0, bl, 255), qBound(0, alpha, 255));
    }
    return QColor();
}

static int parseFontSizeFromToken(QStringView fontSizeToken, bool &ok)
{
    ok = false;
    float size = fontSizeToken.trimmed().toFloat(&ok);
    if (ok) {
        return int(size);
    }
    qWarning().nospace() << "Context2D: A font size of " << fontSizeToken << " is invalid.";
    return 0;
}

/*
    Attempts to set the font size of \a font to \a fontSizeToken, returning
    \c true if successful. If the font size is invalid, \c false is returned
    and a warning is printed.
*/
static bool setFontSizeFromToken(QFont &font, QStringView fontSizeToken)
{
    const QStringView trimmedToken = fontSizeToken.trimmed();
    const QStringView unitStr = trimmedToken.right(2);
    const QStringView value = trimmedToken.left(trimmedToken.size() - 2);
    bool ok = false;
    int size = 0;
    if (unitStr == QStringLiteral("px")) {
        size = parseFontSizeFromToken(value, ok);
        if (ok) {
            font.setPixelSize(size);
            return true;
        }
    } else if (unitStr == QStringLiteral("pt")) {
        size = parseFontSizeFromToken(value, ok);
        if (ok) {
            font.setPointSize(size);
            return true;
        }
    } else {
        qWarning().nospace() << "Context2D: Invalid font size unit in font string.";
    }
    return false;
}


/*
    Returns a list of all of the families in \a fontFamiliesString, where
    each family is separated by spaces. Families with spaces in their name
    must be quoted.
*/
static QStringList extractFontFamiliesFromString(QStringView fontFamiliesString)
{
    QStringList extractedFamilies;
    int quoteIndex = -1;
    QString currentFamily;
    for (int index = 0; index < fontFamiliesString.size(); ++index) {
        const QChar ch = fontFamiliesString.at(index);
        if (ch == u'"' || ch == u'\'') {
            if (quoteIndex == -1) {
                quoteIndex = index;
            } else {
                if (ch == fontFamiliesString.at(quoteIndex)) {
                    // Found the matching quote. +1/-1 because we don't want the quote as part of the name.
                    const QString family = fontFamiliesString.mid(quoteIndex + 1, index - quoteIndex - 1).toString();
                    extractedFamilies.push_back(family);
                    currentFamily.clear();
                    quoteIndex = -1;
                } else {
                    qWarning().nospace() << "Context2D: Mismatched quote in font string.";
                    return QStringList();
                }
            }
        } else if (ch == u' ' && quoteIndex == -1) {
            // This is a space that's not within quotes...
            if (!currentFamily.isEmpty()) {
                // and there is a current family; consider it the end of the current family.
                extractedFamilies.push_back(currentFamily);
                currentFamily.clear();
            } // else: ignore the space
        } else {
            currentFamily.push_back(ch);
        }
    }
    if (!currentFamily.isEmpty()) {
        if (quoteIndex == -1) {
            // This is the end of the string, so add this family to our list.
            extractedFamilies.push_back(currentFamily);
        } else {
            qWarning().nospace() << "Context2D: Unclosed quote in font string.";
            return QStringList();
        }
    }
    if (extractedFamilies.isEmpty()) {
        qWarning().nospace() << "Context2D: Missing or misplaced font family in font string"
                             << " (it must come after the font size).";
    }
    return extractedFamilies;
}

/*
    Tries to set a family on \a font using the families provided in \a fontFamilyTokens.

    The list is ordered by preference, with the first family having the highest preference.
    If the first family is invalid, the next family in the list is evaluated.
    This process is repeated until a valid font is found (at which point the function
    will return \c true and the family set on \a font) or there are no more
    families left, at which point a warning is printed and \c false is returned.
*/
static bool setFontFamilyFromTokens(QFont &font, const QStringList &fontFamilyTokens)
{
    for (const QString &fontFamilyToken : fontFamilyTokens) {
        if (QFontDatabase::hasFamily(fontFamilyToken)) {
            font.setFamily(fontFamilyToken);
            return true;
        } else {
            // Can't find a family matching this name; if it's a generic family,
            // try searching for the default family for it by using style hints.
            int styleHint = -1;
            if (fontFamilyToken.compare(QStringLiteral("serif")) == 0) {
                styleHint = QFont::Serif;
            } else if (fontFamilyToken.compare(QStringLiteral("sans-serif")) == 0) {
                styleHint = QFont::SansSerif;
            } else if (fontFamilyToken.compare(QStringLiteral("cursive")) == 0) {
                styleHint = QFont::Cursive;
            } else if (fontFamilyToken.compare(QStringLiteral("monospace")) == 0) {
                styleHint = QFont::Monospace;
            } else if (fontFamilyToken.compare(QStringLiteral("fantasy")) == 0) {
                styleHint = QFont::Fantasy;
            }
            if (styleHint != -1) {
                QFont tmp;
                tmp.setStyleHint(static_cast<QFont::StyleHint>(styleHint));
                font.setFamily(tmp.defaultFamily());
                return true;
            }
        }
    }
    qWarning("Context2D: The font families specified are invalid: %s", qPrintable(fontFamilyTokens.join(QString()).trimmed()));
    return false;
}

#define Q_TRY_SET_TOKEN(token, value, setStatement) \
if (!(usedTokens & token)) { \
            usedTokens |= token; \
            setStatement; \
} else { \
            qWarning().nospace() << "Context2D: Duplicate token " << QStringLiteral(value) << " found in font string."; \
            return currentFont; \
}

/*
    Parses a font string based on the CSS shorthand font property.

    See: http://www.w3.org/TR/css3-fonts/#font-prop
*/
QFont QCanvas2DUtils::qFontFromString(const QString& fontString, const QFont &currentFont) {
    if (fontString.isEmpty()) {
        qWarning().nospace() << "Context2D: Font string is empty.";
        return currentFont;
    }

    // We know that font-size must be specified and it must be before font-family
    // (which could potentially have "px" or "pt" in its name), so extract it now.
    int fontSizeEnd = fontString.indexOf(QStringLiteral("px"));
    if (fontSizeEnd == -1)
        fontSizeEnd = fontString.indexOf(QStringLiteral("pt"));
    if (fontSizeEnd == -1) {
        qWarning().nospace() << "Context2D: Invalid font size unit in font string.";
        return currentFont;
    }

    int fontSizeStart = fontString.lastIndexOf(u' ', fontSizeEnd);
    if (fontSizeStart == -1) {
        // The font size might be the first token in the font string, which is OK.
        // Regardless, we'll find out if the font is invalid with setFontSizeFromToken().
        fontSizeStart = 0;
    } else {
        // Don't want to take the leading space.
        ++fontSizeStart;
    }

    // + 2 for the unit, +1 for the space that we require.
    fontSizeEnd += 3;

    QFont newFont;
    if (!setFontSizeFromToken(newFont, QStringView{fontString}.mid(fontSizeStart, fontSizeEnd - fontSizeStart)))
        return currentFont;

    // We don't want to parse the size twice, so remove it now.
    QString remainingFontString = fontString;
    remainingFontString.remove(fontSizeStart, fontSizeEnd - fontSizeStart);
    QStringView remainingFontStringRef(remainingFontString);

    // Next, we have to take any font families out, as QString::split() will ruin quoted family names.
    const QStringView fontFamiliesString = remainingFontStringRef.mid(fontSizeStart);
    remainingFontStringRef.truncate(fontSizeStart);
    QStringList fontFamilies = extractFontFamiliesFromString(fontFamiliesString);
    if (fontFamilies.isEmpty()) {
        return currentFont;
    }
    if (!setFontFamilyFromTokens(newFont, fontFamilies))
        return currentFont;

    // Now that we've removed the messy parts, we can split the font string on spaces.
    const QStringView trimmedTokensStr = remainingFontStringRef.trimmed();
    if (trimmedTokensStr.isEmpty()) {
        // No optional properties.
        return newFont;
    }
    const auto tokens = trimmedTokensStr.split(QLatin1Char(' '));

    int usedTokens = NoTokens;
    // Optional properties can be in any order, but font-size and font-family must be last.
    for (const QStringView &token : tokens) {
        if (token.compare(QStringLiteral("normal")) == 0) {
            if (!(usedTokens & FontStyle) || !(usedTokens & FontVariant) || !(usedTokens & FontWeight)) {
                // Could be font-style, font-variant or font-weight.
                if (!(usedTokens & FontStyle)) {
                    // QFont::StyleNormal is the default for QFont::style.
                    usedTokens = usedTokens | FontStyle;
                } else if (!(usedTokens & FontVariant)) {
                    // QFont::MixedCase is the default for QFont::capitalization.
                    usedTokens |= FontVariant;
                } else if (!(usedTokens & FontWeight)) {
                    // QFont::Normal is the default for QFont::weight.
                    usedTokens |= FontWeight;
                }
            } else {
                qWarning().nospace() << "Context2D: Duplicate token \"normal\" found in font string.";
                return currentFont;
            }
        } else if (token.compare(QStringLiteral("bold")) == 0) {
            Q_TRY_SET_TOKEN(FontWeight, "bold", newFont.setBold(true))
        } else if (token.compare(QStringLiteral("italic")) == 0) {
            Q_TRY_SET_TOKEN(FontStyle, "italic", newFont.setStyle(QFont::StyleItalic))
        } else if (token.compare(QStringLiteral("oblique")) == 0) {
            Q_TRY_SET_TOKEN(FontStyle, "oblique", newFont.setStyle(QFont::StyleOblique))
        } else if (token.compare(QStringLiteral("small-caps")) == 0) {
            Q_TRY_SET_TOKEN(FontVariant, "small-caps", newFont.setCapitalization(QFont::SmallCaps))
        } else {
            bool conversionOk = false;
            int weight = token.toInt(&conversionOk);
            if (conversionOk) {
                Q_TRY_SET_TOKEN(FontWeight, "<font-weight>",
                                newFont.setWeight(QFont::Weight(weight)))
            } else {
                // The token is invalid or in the wrong place/order in the font string.
                qWarning().nospace() << "Context2D: Invalid or misplaced token " << token
                                     << " found in font string.";
                return currentFont;
            }
        }
    }
    return newFont;
}

QT_END_NAMESPACE
