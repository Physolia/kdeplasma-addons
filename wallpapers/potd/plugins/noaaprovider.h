/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NOAAPROVIDER_H
#define NOAAPROVIDER_H

#include "potdprovider.h"

class KJob;

/**
 * This class provides the image for NOAA Environmental Visualization Laboratory
 * Image Of the Day
 * located at https://www.nesdis.noaa.gov/content/imagery-and-data.
 */
class NOAAProvider : public PotdProvider
{
    Q_OBJECT

public:
    NOAAProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * Destroys the NOAA provider.
     */
    ~NOAAProvider() override;

private:
    void listPageRequestFinished(KJob *job);
    void pageRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);
};

#endif
