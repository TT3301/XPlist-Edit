#include <QRegularExpression>
#include "myhighlighter.h"

extern int red;

MyHighLighter::MyHighLighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    if (red > 55)
        keywordFormat.setForeground(Qt::blue);
    else
        keywordFormat.setForeground(Qt::green);

    //keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns
        << "\\barray\\b"
        << "\\bdict\\b"
        << "\\bdate\\b"
        << "\\breal\\b"
        << "\\bkey\\b"

        << "\\binteger\\b"
        << "\\bbool\\b"
        << "\\bstring\\b"
        << "\\bdata\\b"
        << "\\b<\\b"
        << "\\b>\\b"
        << "\\b/\\b";

    foreach (const QString& pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    if (red > 55)
        quotationFormat.setForeground(Qt::blue);
    else
        quotationFormat.setForeground(Qt::green);
    rule.pattern = QRegularExpression("/");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression("<");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(">");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);
    quotationFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("#");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void MyHighLighter::highlightBlock(const QString& text)
{
    for (const HighlightingRule& rule : highlightingRules) {
        auto matches = rule.pattern.globalMatch(text);
        while (matches.hasNext()) {
            const auto match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);
    int startIndex = previousBlockState() == 1
        ? 0 : commentStartExpression.match(text).capturedStart();
    while (startIndex >= 0) {
        const auto endMatch = commentEndExpression.match(text, startIndex);
        const int commentLength = endMatch.hasMatch()
            ? endMatch.capturedEnd() - startIndex : text.length() - startIndex;
        if (!endMatch.hasMatch()) setCurrentBlockState(1);
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.match(text, startIndex + commentLength).capturedStart();
    }
}
