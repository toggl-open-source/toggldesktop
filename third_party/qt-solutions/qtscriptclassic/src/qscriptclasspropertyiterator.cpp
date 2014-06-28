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


#include "qscriptclasspropertyiterator.h"


#include "qscriptclasspropertyiterator_p.h"
#include "qscriptstring.h"

QT_BEGIN_NAMESPACE

/*!
  \since 4.4
  \class QScriptClassPropertyIterator

  \brief The QScriptClassPropertyIterator class provides an iterator interface for custom Qt Script objects.

  \ingroup script

  This class is only relevant if you have subclassed QScriptClass and
  want to provide enumeration of your custom properties (e.g. when
  objects of your class are used with QScriptValueIterator, or with
  the for-in statement in scripts).

  The object() function returns the Qt Script object the iterator is
  traversing.

  toFront(), hasNext() and next() provide forward iteration.

  toBack(), hasPrevious() and previous() provide backward iteration.

  name(), id() and flags() return information about the last property
  that was jumped over using next() or previous().

  \sa QScriptClass::newIterator(), QScriptValueIterator
*/

QScriptClassPropertyIteratorPrivate::QScriptClassPropertyIteratorPrivate(QScriptClassPropertyIterator *q)
    : q_ptr(q)
{
}

QScriptClassPropertyIteratorPrivate::~QScriptClassPropertyIteratorPrivate()
{
}

/*!
  Constructs an iterator for traversing \a object.

  Subclasses should ensure that the iterator is set to the front of the
  sequence of properties (before the first property).
*/
QScriptClassPropertyIterator::QScriptClassPropertyIterator(const QScriptValue &object)
    : d_ptr(new QScriptClassPropertyIteratorPrivate(this))
{
    d_ptr->object = object;
}

/*!
  \internal
*/
QScriptClassPropertyIterator::QScriptClassPropertyIterator(const QScriptValue &object,
                                           QScriptClassPropertyIteratorPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    d_ptr->object = object;
}

/*!
  Destroys the iterator.
*/
QScriptClassPropertyIterator::~QScriptClassPropertyIterator()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns the Qt Script object this iterator is traversing.
*/
QScriptValue QScriptClassPropertyIterator::object() const
{
    Q_D(const QScriptClassPropertyIterator);
    return d->object;
}

/*!
  \fn bool QScriptClassPropertyIterator::hasNext() const

  Returns true if there is at least one item ahead of the iterator
  (i.e. the iterator is \e not at the back of the property sequence);
  otherwise returns false.

  \sa next(), hasPrevious()
*/

/*!
  \fn void QScriptClassPropertyIterator::next()

  Advances the iterator by one position.

  Calling this function on an iterator located at the back of the
  container leads to undefined results.

  \sa hasNext(), previous(), name()
*/

/*!
  \fn bool QScriptClassPropertyIterator::hasPrevious() const

  Returns true if there is at least one item behind the iterator
  (i.e. the iterator is \e not at the front of the property sequence);
  otherwise returns false.

  \sa previous(), hasNext()
*/

/*!
  \fn void QScriptClassPropertyIterator::previous()

  Moves the iterator back by one position.

  Calling this function on an iterator located at the front of the
  container leads to undefined results.

  \sa hasPrevious(), next(), name()
*/

/*!
  \fn void QScriptClassPropertyIterator::toFront()

  Moves the iterator to the front of the QScriptValue (before the
  first property).

  \sa toBack(), next()
*/

/*!
  \fn void QScriptClassPropertyIterator::toBack()

  Moves the iterator to the back of the QScriptValue (after the
  last property).

  \sa toFront(), previous()
*/

/*!
  \fn QScriptString QScriptClassPropertyIterator::name() const

  Returns the name of the last property that was jumped over using
  next() or previous().

  \sa id()
*/

/*!
  \fn uint QScriptClassPropertyIterator::id() const

  Returns the id of the last property that was jumped over using
  next() or previous().

  The default implementation returns 0.

  \sa name()
*/
uint QScriptClassPropertyIterator::id() const
{
    return 0;
}

/*!
  Returns the flags of the last property that was jumped over using
  next() or previous().

  The default implementation calls the propertyFlags() function of
  object() with argument name().
*/
QScriptValue::PropertyFlags QScriptClassPropertyIterator::flags() const
{
    return object().propertyFlags(name());
}

QT_END_NAMESPACE

