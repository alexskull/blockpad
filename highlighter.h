/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QSettings>
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE
static bool isLeftBrackets(QChar symbol)
{
    return symbol == '('
            || symbol == '{'
            || symbol == '[';
}

static bool isRightBrackets(QChar symbol)
{
    return symbol == ')'
            || symbol == '}'
            || symbol == ']';
}
struct UBracketInfo
{
    QChar character;
    int position;
};

class TextBlockUserData : public QTextBlockUserData
{
public:
    TextBlockUserData(QString time, int revision)
    {
        _time = time;
        _revision = revision;
    }
    QString time() {return _time;}
    int revision() {return _revision;}
    QVector <UBracketInfo *> brackets()
    { return m_brackets; }
    void insert(UBracketInfo *info)
    {
        int i = 0;

        while(i < m_brackets.size()
              && info->position > m_brackets.at(i)->position)
            i++;

        m_brackets.insert(i, info);
    }
    void insert(QVector <UBracketInfo *> brackets_in)
    {
        m_brackets = brackets_in;
    }
    ~TextBlockUserData(){}
private:
    QString _time;
    int _revision;
    QVector <UBracketInfo *> m_brackets;
};

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    QSettings settings;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat eventsFormat;
    QTextCharFormat mainKeywordsFormat;
    QTextCharFormat builtInTypesFormat;
    QTextCharFormat mappingFormat;
    QTextCharFormat allLanguageWordsFormat;
    QTextCharFormat operatorsFormat;
    QTextCharFormat msg_blocksFormat;
    QTextCharFormat numbersFormat;
    QTextCharFormat hexFormat;
    enum UBrackets { RoundBrackets, CurlyBraces, SquareBrackets };
    void insertBrackets(QChar leftChar, QChar rightChar,
                        TextBlockUserData *data, QString text);
    void insertBrackets(UBrackets brackets,
                        TextBlockUserData *data, QString text);
};

#endif // HIGHLIGHTER_H
