/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "filesystemnoteloader.h"

#include "note.h"

#include <QDebug>
#include <QStandardPaths>
#include <QUuid>

#include <KDirWatch>

class FileNote : public Note
{
    Q_OBJECT
public:
    FileNote(const QString &path, const QString &id);
    void load();
    void save(const QString &text) override;

private:
    void fileSystemChanged(const QString &path);
    QString m_path;
    KDirWatch *m_watcher;
};

FileSystemNoteLoader::FileSystemNoteLoader()
{
    const QString genericDataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString suffix = QStringLiteral("plasma_notes");
    QDir(genericDataLocation).mkdir(suffix);
    m_notesDir.setPath(genericDataLocation + QDir::separator() + suffix);
}

QStringList FileSystemNoteLoader::allNoteIds()
{
    return m_notesDir.entryList(QStringList{QStringLiteral("*.txt")});
}

void FileSystemNoteLoader::deleteNoteResources(const QString &id)
{
    m_notesDir.remove(id);
}

Note *FileSystemNoteLoader::loadNote(const QString &id)
{
    QString idToUse = id;
    if (id.isEmpty()) {
        // Some file systems don't like braces in file names.
        idToUse = QUuid::createUuid().toString(QUuid::WithoutBraces);
    } else if (QUuid::fromString(idToUse).isNull()) {
        // Migration path for notes created by an old code which had a bug
        // that stripped last two chars of UUID.
        QFile file(m_notesDir.absoluteFilePath(idToUse));
        idToUse = QUuid::createUuid().toString(QUuid::WithoutBraces);
        file.rename(m_notesDir.absoluteFilePath(idToUse));
    }

    FileNote *note = new FileNote(m_notesDir.absoluteFilePath(idToUse), idToUse);
    return note;
}

FileNote::FileNote(const QString &path, const QString &id)
    : Note(id)
    , m_path(path)
    , m_watcher(new KDirWatch(this))
{
    m_watcher->addFile(path);

    connect(m_watcher, &KDirWatch::created, this, &FileNote::fileSystemChanged);
    connect(m_watcher, &KDirWatch::dirty, this, &FileNote::fileSystemChanged);

    load();
}

void FileNote::load()
{
    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setNoteText(QString::fromUtf8(file.readAll()));
    }
}

void FileNote::save(const QString &text)
{
    if (text == noteText()) {
        return;
    }

    m_watcher->removeFile(m_path);

    QFile file(m_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(text.toUtf8());
    } else {
        qWarning() << "Could not write notes to file" << m_path;
    }
    setNoteText(text);

    m_watcher->addFile(m_path);
}

void FileNote::fileSystemChanged(const QString &path)
{
    if (path == m_path) {
        load();
    }
}

#include "filesystemnoteloader.moc"
