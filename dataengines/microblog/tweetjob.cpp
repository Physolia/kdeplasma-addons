/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009-2010 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright 2012 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "tweetjob.h"
#include "timelineservice.h"
#include "timelinesource.h"


#include <KDebug>
#include <KIO/Job>

#include "koauth.h"

Q_DECLARE_METATYPE(Plasma::DataEngine::Data)


TweetJob::TweetJob(TimelineSource *source, const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(source->account(), operation, parameters, parent),
      m_url(source->serviceBaseUrl()),
      m_parameters(parameters),
      m_source(source)
{
    if (operation == "statuses/retweet" ||
        operation == "favorites/create" ||
        operation == "favorites/destroy") {
        m_url.setPath(m_url.path()+QString("%1/%2.xml").arg(operation).arg(parameters.value("id").toString()));
        kDebug() << "Operation" << operation << m_url;
    } else if (operation == "update") {
        m_url.setPath(m_url.path()+QString("statuses/%1.xml").arg(operation));
        kDebug() << "Updating status" << m_url;

    } else {
        m_url.setPath(m_url.path()+operation+".xml");
    }
}

void TweetJob::start()
{
    QByteArray data;

    data = "source=kdemicroblog";
    {
        QMapIterator<QString, QVariant> i(m_parameters);
        while (i.hasNext()) {
            i.next();
            if (!i.value().toString().isEmpty()) {
                if (i.key() == "status") {
                    data = data.append("&status=" + i.value().toString().toUtf8().toPercentEncoding());
                } else {
                    data = data.append(QString("&"+i.key()+"="+i.value().toString()).toLatin1());
                }
            }
        }
    }
    KIO::Job *job = KIO::http_post(m_url, data, KIO::HideProgressInfo);

    QOAuth::ParamMap params;
    params.insert("source", "kdemicroblog");

    {
        QMapIterator<QString, QVariant> i(m_parameters);
        while (i.hasNext()) {
            i.next();
            if (!i.value().toString().isEmpty()) {
                if (i.key() == "status") {
                    params.insert("status", i.value().toString().toUtf8().toPercentEncoding());
                    data = data.append("&status=" + i.value().toString().toUtf8().toPercentEncoding());
                } else {
                    params.insert(i.key().toLatin1(), i.value().toString().toLatin1());
                    data = data.append(QString("&"+i.key()+"="+i.value().toString()).toLatin1());
                }
            }
        }
    }
    m_source->oAuthHelper()->sign(job, m_url.pathOrUrl(), params, KOAuth::POST);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(result(KJob*)));
}

void TweetJob::result(KJob *job)
{
    kDebug() << "Job returned... e:" << job->errorText();
    setError(job->error());
    setErrorText(job->errorText());
    setResult(!job->error());
}


#include <tweetjob.moc>

