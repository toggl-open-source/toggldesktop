/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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


#ifndef QSCRIPTLEXER_P_H
#define QSCRIPTLEXER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>


QT_BEGIN_NAMESPACE

class QScriptEnginePrivate;
class QScriptNameIdImpl;

namespace QScript {

class Lexer
{
public:
    Lexer(QScriptEnginePrivate *eng);
    ~Lexer();

    void setCode(const QString &c, int lineno);
    int lex();

    int currentLineNo() const { return yylineno; }
    int currentColumnNo() const { return yycolumn; }

    int startLineNo() const { return startlineno; }
    int startColumnNo() const { return startcolumn; }

    int endLineNo() const { return currentLineNo(); }
    int endColumnNo() const
    { int col = currentColumnNo(); return (col > 0) ? col - 1 : col; }

    bool prevTerminator() const { return terminator; }

    enum State { Start,
                 Identifier,
                 InIdentifier,
                 InSingleLineComment,
                 InMultiLineComment,
                 InNum,
                 InNum0,
                 InHex,
                 InOctal,
                 InDecimal,
                 InExponentIndicator,
                 InExponent,
                 Hex,
                 Octal,
                 Number,
                 String,
                 Eof,
                 InString,
                 InEscapeSequence,
                 InHexEscape,
                 InUnicodeEscape,
                 Other,
                 Bad };

    enum Error {
        NoError,
        IllegalCharacter,
        UnclosedStringLiteral,
        IllegalEscapeSequence,
        IllegalUnicodeEscapeSequence,
        UnclosedComment,
        IllegalExponentIndicator,
        IllegalIdentifier
    };

    enum ParenthesesState {
        IgnoreParentheses,
        CountParentheses,
        BalancedParentheses
    };

    enum RegExpBodyPrefix {
        NoPrefix,
        EqualPrefix
    };

    bool scanRegExp(RegExpBodyPrefix prefix = NoPrefix);

    QScriptNameIdImpl *pattern;
    int flags;

    State lexerState() const
        { return state; }

    QString errorMessage() const
        { return errmsg; }
    void setErrorMessage(const QString &err)
        { errmsg = err; }
    void setErrorMessage(const char *err)
        { setErrorMessage(QString::fromLatin1(err)); }

    Error error() const
        { return err; }
    void clearError()
        { err = NoError; }

private:
    QScriptEnginePrivate *driver;
    int yylineno;
    bool done;
    char *buffer8;
    QChar *buffer16;
    uint size8, size16;
    uint pos8, pos16;
    bool terminator;
    bool restrKeyword;
    // encountered delimiter like "'" and "}" on last run
    bool delimited;
    int stackToken;

    State state;
    void setDone(State s);
    uint pos;
    void shift(uint p);
    int lookupKeyword(const char *);

    bool isWhiteSpace() const;
    bool isLineTerminator() const;
    bool isHexDigit(ushort c) const;
    bool isOctalDigit(ushort c) const;

    int matchPunctuator(ushort c1, ushort c2,
                         ushort c3, ushort c4);
    ushort singleEscape(ushort c) const;
    ushort convertOctal(ushort c1, ushort c2,
                         ushort c3) const;
public:
    static unsigned char convertHex(ushort c1);
    static unsigned char convertHex(ushort c1, ushort c2);
    static QChar convertUnicode(ushort c1, ushort c2,
                                 ushort c3, ushort c4);
    static bool isIdentLetter(ushort c);
    static bool isDecimalDigit(ushort c);

    inline int ival() const { return qsyylval.ival; }
    inline double dval() const { return qsyylval.dval; }
    inline QScriptNameIdImpl *ustr() const { return qsyylval.ustr; }

    const QChar *characterBuffer() const { return buffer16; }
    int characterCount() const { return pos16; }

private:
    void record8(ushort c);
    void record16(QChar c);
    void recordStartPos();

    int findReservedWord(const QChar *buffer, int size) const;

    void syncProhibitAutomaticSemicolon();

    const QChar *code;
    uint length;
    int yycolumn;
    int startlineno;
    int startcolumn;
    int bol;     // begin of line

    union {
        int ival;
        double dval;
        QScriptNameIdImpl *ustr;
    } qsyylval;

    // current and following unicode characters
    ushort current, next1, next2, next3;

    struct keyword {
        const char *name;
        int token;
    };

    QString errmsg;
    Error err;

    bool wantRx;
    bool check_reserved;

    ParenthesesState parenthesesState;
    int parenthesesCount;
    bool prohibitAutomaticSemicolon;
};

} // namespace QScript

QT_END_NAMESPACE


#endif
