/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <david@davidedmundson.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#ifndef ABSTRACTNOTELOADER_H
#define ABSTRACTNOTELOADER_H

class QStringList;
class QString;
class Note;

class AbstractNoteLoader
{
public:
    explicit AbstractNoteLoader();
    virtual ~AbstractNoteLoader();

    virtual QStringList allNoteIds() = 0;
    // Load a note specified by its ID.
    //
    // Important: A loader may decide to change note's ID, so the returned one
    // might have different ID, and if it does -- the old ID must be replaced.
    virtual Note *loadNote(const QString &id) = 0;
    virtual void deleteNoteResources(const QString &id) = 0;

private:
};

#endif // ABSTRACTNOTEMANAGER_H
