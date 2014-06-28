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

#include <QApplication>
#include "population.h"

Population::Population(const QString &country, QObject *parent)
    : QObject(parent), http(this)
{
    // Connect the HTTP transport's responseReady() signal.
    connect(&http, SIGNAL(responseReady()), this, SLOT(getResponse()));

    // Construct a method request message.
    QtSoapMessage request;

    // Set the method and add one argument.
    request.setMethod("getPopulation", "http://www.abundanttech.com/WebServices/Population");
    request.addMethodArgument("strCountry", "", country);

    // Submit the request the the web service.
    http.setHost("www.abundanttech.com");
    http.setAction("http://www.abundanttech.com/WebServices/Population/getPopulation");
    http.submitRequest(request, "/WebServices/Population/population.asmx");

    qDebug("Looking up population of %s...", country.toLatin1().constData());
}

void Population::getResponse()
{
    // Get a reference to the response message.
    const QtSoapMessage &message = http.getResponse();

    // Check if the response is a SOAP Fault message
    if (message.isFault()) {
        qDebug("Error: %s", message.faultString().value().toString().toLatin1().constData());
    }
    else {
        // Get the return value, and print the result.
        const QtSoapType &response = message.returnValue();
        qDebug("%s has a population of %s (last updated at %s)",
               response["Country"].value().toString().toLatin1().constData(),
               response["Pop"].value().toString().toLatin1().constData(),
               response["Date"].value().toString().toLatin1().constData());
    }
    qApp->quit();
}
