/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "AboutData.h"

AboutData::AboutData()
    : KAboutData(
            QByteArray("lancelot"),
            QByteArray(""),
            ki18n("Lancelot"),
            QByteArray("1.6"),
            ki18n("Next-generation application launcher."),
            KAboutData::License_GPL,
            ki18n("(C) 2007-2009 Ivan Čukić"),
            KLocalizedString()
        )
{
    setHomepage("http://lancelot.fomentgroup.org/");

    addAuthor(
            ki18n("Ivan Čukić"),
            ki18n("Maintainer and main developer"),
            "ivan(at)fomentgroup.org",
            "http://ivan.fomentgroup.org"
            );

    addCredit(
            ki18n("Siraj Razick"),
            ki18n("Raptor compositing code"));
    addCredit(
            ki18n("Robert Knight"),
            ki18n("Kickoff data models"));
}

