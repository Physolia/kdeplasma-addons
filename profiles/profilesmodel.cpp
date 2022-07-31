/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "profilesmodel.h"

// KF
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <KFileUtils>
#include <KLocalizedString>
// Qt
#include <KIO/CommandLauncherJob>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

ProfilesModel::ProfilesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ProfilesModel::init()
{
    m_dirWatch = new KDirWatch(this);
    const QStringList konsoleDataBaseDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &konsoleDataBaseDir : konsoleDataBaseDirs) {
        m_dirWatch->addDir(konsoleDataBaseDir + QLatin1Char('/') + m_appName);
    }
    connect(m_dirWatch, &KDirWatch::dirty, this, &ProfilesModel::loadProfiles);

    loadProfiles();
}

void ProfilesModel::loadProfiles()
{
    beginResetModel();
    m_data.clear();

    QStringList profilesPaths;
    if (m_appName == QLatin1String("kate")) {
        const QDir sessionsDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kate/sessions"));
        profilesPaths = sessionsDir.entryList({QStringLiteral("*.katesession")}, QDir::Files, QDir::Name);
        m_data << ProfileData{i18n("Start Kate (no arguments)"), QString(), m_appName};
        m_data << ProfileData{i18n("New Kate Session"), QString(), QStringLiteral("document-new")};
    } else {
        const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, m_appName, QStandardPaths::LocateDirectory);
        profilesPaths = KFileUtils::findAllUniqueFiles(dirs, {QStringLiteral("*.profile")});
    }

    for (const auto &profilePath : std::as_const(profilesPaths)) {
        QFileInfo info(profilePath);
        const QString profileIdentifier = QUrl::fromPercentEncoding(info.baseName().toLatin1());
        QString name = profileIdentifier;
        QString iconName;
        if (m_appName == QLatin1String("konsole")) {
            KConfig cfg(profilePath, KConfig::SimpleConfig);
            KConfigGroup grp(&cfg, "General");
            iconName = grp.readEntry("Icon", iconName);
            name = grp.readEntry("Name", name);
        } else {
            iconName = m_appName;
        }

        m_data.append(ProfileData{name, profileIdentifier, iconName});
    }
    endResetModel();
}

QHash<int, QByteArray> ProfilesModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {ProfileIdentifierRole, "profileIdentifier"},
        {IconNameRole, "iconName"},
    };
}

QVariant ProfilesModel::data(const QModelIndex &index, int role) const
{
    const ProfileData &data = m_data.at(index.row());
    switch (role) {
    case NameRole:
        return data.name;
    case ProfileIdentifierRole:
        return data.profileIdentifier;
    case IconNameRole:
        return data.iconName;
    default:
        return QVariant();
    }
}

void ProfilesModel::openProfile(const QString profileIdentifier)
{
    KIO::CommandLauncherJob *job;
    if (m_appName == QLatin1String("konsole")) {
        job = new KIO::CommandLauncherJob(m_appName, QStringList{QStringLiteral("--profile"), profileIdentifier});
        job->setDesktopName(QStringLiteral("org.kde.konsole"));
    } else if (m_appName == QLatin1String("kate")) {
        QStringList args;
        if (!profileIdentifier.isEmpty()) {
            args << QStringLiteral("--start") << profileIdentifier;
        }
        args << QStringLiteral("-n");
        job = new KIO::CommandLauncherJob(m_appName, args);
        job->setDesktopName(QStringLiteral("org.kde.kate"));
    } else {
        Q_UNREACHABLE();
    }

    job->start();
}