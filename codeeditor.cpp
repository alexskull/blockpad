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

#include <QtWidgets>
#include <QDateTime>
#include <QLocale>
#include <QDebug>
#include <QApplication>
#include <QFontMetrics>
#include <QString>
#include <QStringList>
#include <QCursor>
#include "codeeditor.h"
#include "global.h"


CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()),
             this, SLOT(matchBrackets()));  
    init();
}

void CodeEditor::slotFindAllCurrentFile()
{
    QMap<QString, QVector<SearchItem>> res;
    //fill res
    {
        auto finds = fillFindResults(currentName);
        if(!finds.isEmpty())
            res[currentName] = finds;
    }
    emit sigFindResults(res);
}

void CodeEditor::slotFindAllAllFiles()
{
    QMap<QString, QVector<SearchItem>> res;
    //fill res
    foreach(auto nameFile, allDocuments.keys())
    {
        allHighlighters[nameFile]->markSearch(strSearch);
        auto finds = fillFindResults(nameFile);
        if(!finds.isEmpty())
            res[nameFile] = finds;
    }
    emit sigFindResults(res);
}

void CodeEditor::slotSearchMark(QString strCurrent)
{
    strSearch =strCurrent;
    int firstFinding = allHighlighters[currentName]->markSearch(strCurrent);
    if(firstFinding >= 0)
    {
        auto tCursor = textCursor();
        tCursor.setPosition(firstFinding);
        setTextCursor(tCursor);
    }
    highlightCurrentLine();
}

QVector<SearchItem> CodeEditor::fillFindResults(QString nameFile)
{
    QVector<SearchItem> res;
    QTextBlock block = allDocuments[nameFile]->firstBlock();
    while(block.isValid())
    {
        auto userData = static_cast <TextBlockUserData *> (
                        block.userData());
        if(userData != nullptr)
        {
            auto finds = userData->finds();
            if(!finds.isEmpty())
            {
                for(int i=0; i<finds.size(); i++)
                {
                    SearchItem item;
                    item.numLine = block.blockNumber()+1;
                    item.strLine = block.text();
                    item.length = finds[i]->length;
                    item.start = finds[i]->posStart;
                    res.append(item);
                }
            }
        }
        block = block.next();
    }
    return res;
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return
            &&
        textCursor().blockNumber() == 0)
    {
        moveCursor (QTextCursor::Start);
        insertPlainText ("\n");
        moveCursor (QTextCursor::Start);
        allHighlighters[currentName]->rehighlightBlock(
                    document()->findBlockByLineNumber(1));
        event->ignore();
    }
    else
    {
        event->accept();
        QPlainTextEdit::keyPressEvent(event);
    }
}

void CodeEditor::init()
{
    calcLineNumberAreaWidth();
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setUndoRedoEnabled(true);
}

void CodeEditor::slotHighlightingCode(bool on)
{
    allHighlighters[currentName]->rehighlight();
}

QByteArray CodeEditor::dataToEncrypt()
{
    QByteArray res;
    int numDocs = allDocuments.size();
    res.append((const char *)&numDocs, sizeof(int));
    foreach(auto nameDocument, allDocuments.keys())
    {
        bool bVisible = false;
        res.append((const char *)&bVisible, sizeof(bool));
        int nameDocumentSize = nameDocument.size();
        res.append((const char *)&nameDocumentSize, sizeof(int));
        res.append(nameDocument);
        auto doc= allDocuments[nameDocument];
        if(doc == document())
            bVisible = true;
        auto allText = doc->toPlainText().toUtf8();
        int length = allText.size();
        res.append((const char *)&length, sizeof(int));
        res.append(allText);
        //timestamps
        {
            QTextBlock block = doc->firstBlock();
            while (block.isValid())
            {
                QString time;
                //fill time
                {
                    TextBlockUserData* timeUserData =  dynamic_cast<TextBlockUserData*>(block.userData());
                    if(!timeUserData)
                        time = QLocale("en_EN").toString(QDateTime::currentDateTime(), "hh:mm ap M/d/yyyy");
                    else
                        time = ((TextBlockUserData*)block.userData())->time();
                }
                int sizeTime = time.size();
                res.append((const char *)&sizeTime, sizeof(int));
                res.append(time);
                block = block.next();
            }
        }
    }
    return res;
}

void CodeEditor::renameDocument(QString newName, QString oldName)
{
    allDocuments[newName] = allDocuments[oldName];
    allHighlighters[newName] = allHighlighters[oldName];
    allDocuments.remove(oldName);
    allHighlighters.remove(oldName);
    if(currentName == oldName)
        currentName = newName;
}

QMap<QString, QTextDocument *> CodeEditor::slotLoadData(QByteArray allLoadData, int &pos)
{
    loadFile = true;
    int nDocuments = *((int *)allLoadData.mid(pos, sizeof(int)).data());
    pos+=sizeof(int);

    for(int iDoc=0; iDoc<nDocuments; iDoc++)
    {
        QTextDocument * doc = new QTextDocument(&documentsParent);
        doc->setDocumentLayout(new QPlainTextDocumentLayout(doc));
        bool bVisible = *((bool *)allLoadData.mid(pos, sizeof(bool)).data());
        pos+=sizeof(bool);

        int nameDocumentSize = *((int *)allLoadData.mid(pos, sizeof(int)).data());
        pos+=sizeof(int);
        QString nameDocument = allLoadData.mid(pos, nameDocumentSize);
        pos+=nameDocumentSize;

        int textSize = *((int *)allLoadData.mid(pos, sizeof(int)).data());
        pos+=sizeof(int);

        QString plainText = QString::fromUtf8(allLoadData.mid(pos, textSize));

        doc->setPlainText(plainText);
        pos+=textSize;

        QTextBlock block = doc->firstBlock();
        while (block.isValid())
        {
            int timeSize = *((int *)allLoadData.mid(pos, sizeof(int)).data());
            pos+=sizeof(int);
            QString timeBlock = allLoadData.mid(pos, timeSize);
            TextBlockUserData * userData = new TextBlockUserData(timeBlock, block.revision());
            if(block.userData()!= nullptr)
            {
                userData->insert(((TextBlockUserData*)block.userData())->brackets());
            }
            block.setUserData(userData);
            pos+=timeSize;
            block = block.next();
        }
        allDocuments[nameDocument] = doc;
        allHighlighters[nameDocument] = new Highlighter(doc);
        if(bVisible)
        {
            setDocument(doc);
            currentName = nameDocument;
        }
    }
    init();
    repaint();
    loadFile = false;
    return allDocuments;
}

QMap<QString, QTextDocument *> CodeEditor::getAllDocuments()
{
    return allDocuments;
}

void CodeEditor::removeDocument(QString nameDocument)
{
    if(allDocuments.contains(nameDocument))
    {
        auto doc = allDocuments[nameDocument];
        allDocuments.remove(nameDocument);
        delete doc;
        allHighlighters.remove(nameDocument);
    }
}

void CodeEditor::slotFindNext()
{
    QTextBlock block = textCursor().block();
    bool findNext = false;
    bool wrapAroundStart = false;
    while(1)
    {
        auto userData = static_cast <TextBlockUserData *> (
                        block.userData());
        if(userData != nullptr)
        {
            auto finds = userData->finds();
            for(int i=0; i<finds.size(); i++)
            {
                if(block!= textCursor().block()
                      ||
                   (finds[i]->posStart + block.position()
                        > textCursor().position())
                       ||
                       wrapAroundStart)
                {
                    QTextCursor tcursor = textCursor();
                    tcursor.setPosition(finds[i]->posStart
                                        + block.position());
                    setTextCursor(tcursor);
                    findNext = true;
                    break;
                }
            }
        }
        if(findNext)
            break;
        block = block.next();
        if(!block.isValid())
        {
            if(settings.value(defWrapAround).toBool()
                    && !wrapAroundStart)
            {
                block = document()->firstBlock();
                wrapAroundStart = true;
            }
            else
                break;
        }
    }
}


void CodeEditor::slotCurrentFindResultChanged(QString fileName,
                                              int blockNumber,
                                              int positionResult)
{
    setCurrentDocument(fileName);
    QTextCursor tcursor = textCursor();
    tcursor.setPosition(allDocuments[fileName]->findBlockByNumber(blockNumber).position()
                        + positionResult);
    setTextCursor(tcursor);
}

void CodeEditor::slotFindPrev()
{
    QTextBlock block = textCursor().block();
    bool findPrev = false;
    bool wrapAroundStart = false;
    while(1)
    {
        auto userData = static_cast <TextBlockUserData *> (
                        block.userData());
        if(userData != nullptr)
        {
            auto finds = userData->finds();
            for(int i=finds.size()-1; i>=0; i--)
            {
                if(block!= textCursor().block()
                      ||
                   (finds[i]->posStart + block.position()
                        < textCursor().position())
                      ||
                      wrapAroundStart)
                {
                    QTextCursor tcursor = textCursor();
                    tcursor.setPosition(finds[i]->posStart
                                        + block.position());
                    setTextCursor(tcursor);
                    findPrev = true;
                    break;
                }
            }
        }
        if(findPrev)
            break;
        block = block.previous();
        if(!block.isValid())
        {
            if(settings.value(defWrapAround).toBool()
                    && !wrapAroundStart)
            {
                block = document()->lastBlock();
                wrapAroundStart = true;
            }
            else
                break;
        }
    }
}

void CodeEditor::setCurrentDocument(QString nameDocument)
{
    if(allDocuments.contains(nameDocument))
    {
        if(allDocuments[nameDocument] != document())
        {       
            setDocument(allDocuments[nameDocument]);
            currentName = nameDocument;
            int firstFinding = allHighlighters[currentName]->markSearch(strSearch);
            if(firstFinding >= 0)
            {
                auto tCursor = textCursor();
                tCursor.setPosition(firstFinding);
                setTextCursor(tCursor);
            }
            init();        
        }
    }
    else
    {
        allDocuments[nameDocument] = new QTextDocument(&documentsParent);
        allDocuments[nameDocument]->setDocumentLayout(new QPlainTextDocumentLayout(allDocuments[nameDocument]));
        setDocument(allDocuments[nameDocument]);
        allHighlighters[nameDocument] = new Highlighter(allDocuments[nameDocument]);
        currentName = nameDocument;
        init();
        QTextBlock block = allDocuments[nameDocument]->firstBlock();
        while (block.isValid())
        {
            QString timeBlock = QLocale("en_EN").toString(QDateTime::currentDateTime(), "hh:mm ap M/d/yyyy");
            TextBlockUserData * userData = new TextBlockUserData(timeBlock, block.revision());
            block.setUserData(userData);
            block = block.next();
        }
    }
}

QColor CodeEditor::mix2clr(const QColor &clr1, const QColor &clr2) {
    auto r = (clr1.red() + clr2.red())/2;
    auto g = (clr1.green() + clr2.green())/2;
    auto b = (clr1.blue() + clr2.blue())/2;
    return QColor(r, g, b, qMax(clr1.alpha(), clr2.alpha()));
}

void CodeEditor::calcLineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    spaceLineNumber = 7 + fontMetrics().width(QLatin1Char('9')) * digits;
    _lineNumberAreaWidth = fontMetrics().width("99:99 pm 99/99/9999")
            + spaceLineNumber;
}

int CodeEditor::lineNumberAreaWidth()
{
    calcLineNumberAreaWidth();
    return _lineNumberAreaWidth;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
    {
        QRect rectTextUpdate = QRect(0, rect.y(),
                               lineNumberArea->width(), rect.height());
        lineNumberArea->update(rectTextUpdate);
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::slotReplace(QString replaceStr)
{
    //replace
    {
        auto block = textCursor().block();
        auto userData = static_cast <TextBlockUserData *> (
                        block.userData());
        if(userData != nullptr)
        {
            auto finds = userData->finds();
            for(int i=0; i<finds.size(); i++)
            {
                auto find = finds[i];
                if(find->posStart + block.position()
                   == textCursor().position())
                {
                    auto tCursor = textCursor();
                    tCursor.beginEditBlock();
                    tCursor.setPosition(textCursor().position()
                                        +find->length,
                                        QTextCursor::KeepAnchor);
                    tCursor.insertText(replaceStr);
                    tCursor.endEditBlock();
                    return;
                }
            }
        }
    }
    slotFindNext();
}

void CodeEditor::slotReplaceAllCurrent(QString replaceStr)
{
    replaceInFile(currentName, replaceStr, true);
}

void CodeEditor::slotReplaceAllAll(QString replaceStr)
{
    bool bFirst = true;
    foreach(QString fileName, allDocuments.keys())
    {
        allHighlighters[fileName]->markSearch(strSearch);
        replaceInFile(fileName,
                      replaceStr,
                      bFirst);
        bFirst = false;
    }
}

void CodeEditor::replaceInFile(QString fileName,
                               QString replaceStr,
                               bool bFirstFile)
{
    auto block = allDocuments[fileName]->firstBlock();
    bool bFirstOperation = bFirstFile;
    auto tCursor = QTextCursor(allDocuments[fileName]);
    while(block.isValid())
    {        
        auto userData = static_cast <TextBlockUserData *> (
                        block.userData());
        if(userData != nullptr)
        {
            auto finds = userData->finds();
            if(!finds.isEmpty())
                allHighlighters[fileName]->setHighlightSearch(false);
            for(int i=0; i<finds.size(); i++)
            {
                auto find = finds[i];
                if(bFirstOperation)
                {
                    tCursor.beginEditBlock();
                    bFirstOperation = false;
                }
                else
                    tCursor.joinPreviousEditBlock();
                tCursor.setPosition(find->posStart
                                     + block.position());
                tCursor.setPosition(find->posStart
                                    +find->length
                                     + block.position(),
                                    QTextCursor::KeepAnchor);
                tCursor.insertText(replaceStr);
                tCursor.endEditBlock();
                for(int iM=0; iM<finds.size(); iM++)
                {
                    finds[iM]->posStart +=
                            replaceStr.size() - strSearch.size();
                }
            }
            if(!finds.isEmpty())
            {
                allHighlighters[fileName]->setHighlightSearch(true);
                allHighlighters[fileName]->rehighlightBlock(block);
            }
        }
        block = block.next();
    }
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(175,232,255);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    if(auto userData = static_cast <TextBlockUserData *> (
                textCursor().block().userData()))
    {
        auto findWords = userData->finds();
        foreach(auto find, findWords)
        {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = mix2clr(find->color,
                                       QColor(175,232,255));
            //QColor lineColor = find->color;
            if(find->posStart+ textCursor().block().position()
                    == textCursor().position())
                lineColor = mix2clr(Qt::green,
                                    QColor(175,232,255));
            //    lineColor = QColor(204,255,164);
            selection.format.setBackground(lineColor);
            QTextCursor cursor(textCursor().block());
            cursor.clearSelection();
            cursor.setPosition(find->posStart
                               + textCursor().block().position());
            cursor.setPosition(find->posStart
                               + textCursor().block().position()
                               + find->length,
                               QTextCursor::KeepAnchor);
            selection.cursor = cursor;
            extraSelections.append(selection);
        }
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    {
        QPainter painter(lineNumberArea);
        painter.fillRect(QRect(event->rect().left(),
                               event->rect().top(),
                               spaceLineNumber,
                               event->rect().height()),
                         Qt::lightGray);
        painter.fillRect(QRect(event->rect().left() +spaceLineNumber,
                               event->rect().top(),
                               event->rect().width() -spaceLineNumber,
                               event->rect().height()),
                         QColor("#e3e3e3"));
        {
            //maybe firstBlock?
            QTextBlock block = firstVisibleBlock();
            int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
            int bottom = top + (int) blockBoundingRect(block).height();
            int blockNumber = block.blockNumber();
            while (block.isValid() && top <= event->rect().bottom()) {
                if (bottom >= event->rect().top()) {
                    QString dateTime = QLocale("en_EN").toString(QDateTime::currentDateTime(), "hh:mm ap M/d/yyyy");
                    QString number = QString::number(blockNumber + 1);
                    TextBlockUserData * data = new TextBlockUserData(dateTime, block.revision());          
                    if(!block.userData())
                    {
                        block.setUserData(data);
                        if(!loadFile)
                            emit newChanges();
                    }
                    else
                    {
                        if(((TextBlockUserData*)block.userData())->revision()!= block.revision())
                        {
                            data->insert(((TextBlockUserData*)block.userData())->brackets());
                            data->addFindInfo(((TextBlockUserData*)block.userData())->finds());
                            block.setUserData(data);
                            if(!loadFile)
                                emit newChanges();
                        }
                    }
                    if(block.isVisible() )
                    {
                        painter.setPen(Qt::black);
                        painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                                         Qt::AlignLeft,number);
                        painter.drawText(spaceLineNumber, top, lineNumberArea->width(), fontMetrics().height(),
                                         Qt::AlignLeft,((TextBlockUserData*)block.userData())->time());
                    }
                }

                block = block.next();
                top = bottom;
                bottom = top + (int) blockBoundingRect(block).height();
                ++blockNumber;
            }
        }
    }
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    //highliting 1 line
    {
        QTextBlock block = document()->firstBlock();
        int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
        int bottom = top + (int) blockBoundingRect(block).height();
        if (block.isValid() && top <= event->rect().bottom())
        {
            if (bottom >= event->rect().top())
            {
                QColor color;
                //fill color
                {
                    color = QColor(248,255,255);
                }
                painter.setPen(QPen(color));
                painter.setBrush(QBrush (color));
                painter.drawRect(document()->documentMargin(),
                                 top, viewport()->width() - 2*document()->documentMargin(),
                                 bottom-top );
            }
        }
    }
    QPlainTextEdit::paintEvent(event);
}

void CodeEditor::matchBrackets()
{
    QTextBlock textBlock = textCursor().block();

    TextBlockUserData *data = static_cast <TextBlockUserData *> (textBlock.userData());

    if(data)
    {
        QVector <UBracketInfo *> brackets = data->brackets();
        int position = textCursor().block().position();

        for(int i = 0; i < brackets.size(); i++)
        {
            UBracketInfo *bracket = brackets.at(i);
            int currentPosition = textCursor().position() - textBlock.position();

            // Clicked on a left brackets?
            if (bracket->position == currentPosition
                 && isLeftBrackets(bracket->character))
            {
                if (matchLeftBrackets(textBlock, i + 1, 0))
                    createBracketsSelection(position + bracket->position);
            }

            // Clicked on a right brackets?
            if (bracket->position == currentPosition - 1
                 && isRightBrackets(bracket->character))
            {
                if (matchRightBrackets(textBlock, i - 1, 0))
                    createBracketsSelection(position + bracket->position);
            }
        }
    }
}

/** Test left brackets match **/
bool CodeEditor::matchLeftBrackets(QTextBlock currentBlock, int index, int numberLeftBracket)
{
    while (currentBlock.isValid())
    {
        TextBlockUserData *data = static_cast <TextBlockUserData *> (currentBlock.userData());

        QVector<UBracketInfo *> brackets = data->brackets();

        int positionInDocument = currentBlock.position();

        // Match in same line?
        for (; index < brackets.count(); index++)
        {
            UBracketInfo *bracket = brackets.at(index);

            if (isLeftBrackets(bracket->character))
            {
                ++numberLeftBracket;
                continue;
            }

            if (isRightBrackets(bracket->character)
                 && numberLeftBracket == 0)
            {
                createBracketsSelection(positionInDocument + bracket->position);
                return true;
            }else
                --numberLeftBracket;
        }

        // No match yet? Then try next block
        currentBlock = currentBlock.next();
        index = 0;
//        if (currentBlock.isValid())
//            return matchLeftBrackets(currentBlock, 0, numberLeftBracket);
    }
    // No match at all
    return false;
}

/** Test right brackets match **/
bool CodeEditor::matchRightBrackets(QTextBlock currentBlock, int index, int numberRightBracket)
{
    while(currentBlock.isValid())
    {
        TextBlockUserData *data = static_cast <TextBlockUserData *> (currentBlock.userData());

        QVector<UBracketInfo *> brackets = data->brackets();
        int positionInDocument = currentBlock.position();

        // Match in same line?
        for (int i = index; i >= 0; i--)
        {
            UBracketInfo *bracket = brackets.at(i);

            if (isRightBrackets(bracket->character))
            {
                ++numberRightBracket;
                continue;
            }

            if (isLeftBrackets(bracket->character)
                 && numberRightBracket == 0)
            {
                createBracketsSelection(positionInDocument + bracket->position);
                return true;
            } else
                --numberRightBracket;
        }

        // No match yet? Then try previous block
        currentBlock = currentBlock.previous();
        if (currentBlock.isValid())
        {

            // Recalculate correct index first
            TextBlockUserData *data = static_cast <TextBlockUserData *> (currentBlock.userData());

            QVector <UBracketInfo *> brackets = data->brackets();

            index = brackets.count() - 1;
            //return matchRightBrackets(currentBlock, brackets.count() - 1, numberRightBracket);
        }
    }
    // No match at all
    return false;
}

/** Set brackets highlighter at pos **/
void CodeEditor::createBracketsSelection(int position)
{
    QList <QTextEdit::ExtraSelection> listSelections = extraSelections();

    QTextEdit::ExtraSelection selection;

    QTextCharFormat format = selection.format;
    format.setForeground(Qt::red);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(position);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    selection.cursor = cursor;

    listSelections.append(selection);

    setExtraSelections(listSelections);
}

