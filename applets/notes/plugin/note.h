/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef NOTE_H
#define NOTE_H

#include <QObject>

class Note : public QObject
{
    Q_OBJECT
    // ID looks like a UUID with dashed and without braces. But due to an old
    // bug (2014-2022), the last two hex digits(one octet) of UUID were being
    // stripped away, so it is only safe to treat is as a generic string.
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString noteText READ noteText NOTIFY noteTextChanged)

public:
    explicit Note(const QString &id);
    QString id() const;

    // what's in the plasmoid
    // backends save this and write into storedText
    QString noteText() const;
    void setNoteText(const QString &text);

public Q_SLOTS:
    virtual void save(const QString &text) = 0;

    // FUTURE
    //     status  None, Ready, Loading, Error

Q_SIGNALS:
    void noteTextChanged();

private:
    const QString m_id;
    QString m_noteText;
};

#endif // NOTE_H
