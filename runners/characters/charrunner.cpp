/* SPDX-FileCopyrightText: 2010 Anton Kreuzkamp <akreuzkamp@web.de>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "charrunner.h"
#include "config_keys.h"

// KF
#include <KRunner/QueryMatch>
#include <KLocalizedString>
// Qt
#include <QGuiApplication>
#include <QDebug>
#include <QClipboard>

CharacterRunner::CharacterRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName(QStringLiteral("CharacterRunner"));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
        Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Executable |
        Plasma::RunnerContext::ShellCommand);
}

CharacterRunner::~CharacterRunner()
{
}

void CharacterRunner::reloadConfiguration()
{

    const KConfigGroup grp = config();
    m_triggerWord = grp.readEntry(CONFIG_TRIGGERWORD, DEFAULT_TRIGGERWORD.toString());
    m_aliases = grp.readEntry(CONFIG_ALIASES, QStringList());
    m_codes = grp.readEntry(CONFIG_CODES, QStringList());
    if (m_codes.size() != m_aliases.size()) {
        m_aliases.clear();
        m_codes.clear();
        qWarning() << "Config entries for alias list and code list have different sizes, ignoring all.";
    }

    addSyntax(Plasma::RunnerSyntax(m_triggerWord + QStringLiteral(":q:"),
                                   i18n("Creates Characters from :q: if it is a hexadecimal code or defined alias.")));
}

void CharacterRunner::match(Plasma::RunnerContext &context)
{
    QString term = context.query().remove(QLatin1Char(' '));

    if (term.length() < 2 || !term.startsWith(m_triggerWord) || !context.isValid()) {
        return;
    }

    term = term.remove(0, m_triggerWord.length()); //remove the triggerword

    //replace aliases by their hex.-code
    if (m_aliases.contains(term)) {
        term = m_codes[m_aliases.indexOf(term)];
    }

    bool ok;
    int hex = term.toInt(&ok, 16); //convert query into int
    if (!ok) {
        return;
    }

    //make special character out of the hex.-code
    const QString specChar = QChar(hex);
    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::ExactMatch);
    match.setIconName(QStringLiteral("accessories-character-map"));
    match.setText(specChar);
    match.setData(specChar);
    context.addMatch(match);
}

void CharacterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QGuiApplication::clipboard()->setText(match.data().toString());
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(CharacterRunner, "plasma-runner-character.json")

#include "charrunner.moc"
