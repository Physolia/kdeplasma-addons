/*
 * Copyright (C) 2007,2008 Petri Damstén <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "converterrunner.h"
#include <QApplication>
#include <QClipboard>
#include <QSet>
#include <KIcon>
#include <KDebug>
#include <KToolInvocation>
#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>

#define CONVERSION_CHAR '>'

using namespace KUnitConversion;

class StringParser
{
public:
    enum GetType
    {
        GetString = 1,
        GetDigit  = 2
    };

    StringParser(const QString &s) : m_index(0), m_s(s) {};
    ~StringParser() {};

    QString get(int type)
    {
        QChar current;
        QString result;

        passWhiteSpace();
        while (true) {
            current = next();
            if (current.isNull()) {
                break;
            }
            if (current.isSpace()) {
                break;
            }
            bool number = isNumber(current);
            if (type == GetDigit && !number) {
                break;
            }
            if (type == GetString && number) {
                break;
            }
            if(current == CONVERSION_CHAR) {
                break;
            }
            ++m_index;
            result += current;
        }
        return result;
    }

    bool isNumber(const QChar &ch)
    {
        if (ch.isNumber()) {
            return true;
        }
        if (QString(".,-+").contains(ch)) {
            return true;
        }
        return false;
    }

    QString rest()
    {
        return m_s.mid(m_index).simplified();
    }

    void pass(const QStringList &strings)
    {
        passWhiteSpace();
        const QString temp = m_s.mid(m_index);

        foreach (const QString& s, strings) {
            if (temp.startsWith(s)) {
                m_index += s.length();
                return;
            }
        }
    }

private:
    void passWhiteSpace()
    {
        while (next().isSpace()) {
            ++m_index;
        }
    }

    QChar next()
    {
        if (m_index >= m_s.size()) {
            return QChar::Null;
        }
        return m_s.at(m_index);
    }

    int m_index;
    QString m_s;
};

ConverterRunner::ConverterRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName("Converter");

    m_separators << QString(CONVERSION_CHAR);
    m_separators << i18nc("list of words that can used as amount of 'unit1' [in|to|as] 'unit2'",
                          "in;to;as").split(';');

    //can not ignore commands: we have things like m4
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);

    QString description = i18n("Converts the value of :q: when :q: is made up of "
                               "\"value unit [>, to, as, in] unit\". You can use the "
                               "Unit converter applet to find all available units.");
    addSyntax(Plasma::RunnerSyntax("=:q:", description));
}

ConverterRunner::~ConverterRunner()
{
}

void ConverterRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.size() < 2) {
        return;
    }

    StringParser cmd(term);
    QString unit1;
    QString value;
    QString unit2;

    unit1 = cmd.get(StringParser::GetString);
    value = cmd.get(StringParser::GetDigit);
    if (value.isEmpty()) {
        return;
    }
    if (unit1.isEmpty()) {
        unit1 = cmd.get(StringParser::GetString | StringParser::GetDigit);
        if (unit1.isEmpty()) {
            return;
        }
    }

    const QString s = cmd.get(StringParser::GetString);

    if (!s.isEmpty() && !m_separators.contains(s)) {
        unit1 += ' ' + s;
    }
    if (s.isEmpty() || !m_separators.contains(s)) {
        cmd.pass(m_separators);
    }
    unit2 = cmd.rest();

    Converter converter;
    UnitCategory* category = converter.categoryForUnit(unit1);
    if (!category) {
        return;
    }
    Value v = category->convert(Value(value, unit1), unit2);
    if (v.isValid()) {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIcon(KIcon("edit-copy"));
        match.setText(v.toString());
        match.setData(v.number());
        context.addMatch(term, match);
    } else if (!unit2.isEmpty()) {
        const QStringList units = category->allUnits();
        QSet<UnitPtr> matchingUnits;
        foreach (const QString& s, units) {
            if (s.startsWith(unit2, Qt::CaseInsensitive)) {
                matchingUnits << category->unit(s);
            }
        }
        foreach (const UnitPtr& u, matchingUnits) {
            v = category->convert(Value(value, unit1), u->symbol());
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(KIcon("edit-copy"));
            match.setText(v.toString());
            match.setData(v.number());
            context.addMatch(term, match);
        }
    }
    if (v.isValid() && !v.unit()->category()->description().isEmpty()) {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setIcon(KIcon("document-open-remote"));
        match.setText(v.unit()->category()->description());
        match.setData(v.unit()->category()->url().prettyUrl());
        context.addMatch(term, match);
    }
}

void ConverterRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString data = match.data().toString();
    if (data.startsWith(QLatin1String("http://"))) {
        KToolInvocation::invokeBrowser(data);
    } else {
        QApplication::clipboard()->setText(data);
    }
}

#include "converterrunner.moc"
