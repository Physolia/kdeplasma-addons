/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.19 as Kirigami

import org.kde.plasmacalendar.alternatecalendarconfig 1.0

Kirigami.FormLayout {
    id: configPage

    anchors.left: parent.left
    anchors.right: parent.right

    // expected API
    signal configurationChanged

    // expected API
    function saveConfig() {
        configStorage.calendarSystem = calendarSystemComboBox.currentValue;
        configStorage.dateOffset = dateOffsetSpinBoxLoader.active && dateOffsetSpinBoxLoader.item.value || 0;

        configStorage.save();
    }

    ConfigStorage {
        id: configStorage
    }

    QQC2.ComboBox {
        id: calendarSystemComboBox
        Kirigami.FormData.label: i18ndc("plasma_calendar_alternatecalendar", "@label:listbox", "Calendar system:")
        model: configStorage.calendarSystemModel
        textRole: "display"
        valueRole: "id"
        currentIndex: configStorage.currentIndex
        onActivated: configPage.configurationChanged();
    }

    Loader {
        id: dateOffsetSpinBoxLoader
        active: calendarSystemComboBox.currentValue === "IslamicCivil"
        visible: active
        Kirigami.FormData.label: i18ndc("plasma_calendar_alternatecalendar", "@label:spinbox", "Date offset:")

        sourceComponent: QQC2.SpinBox {
            stepSize: 1
            from: -10
            to: 10
            value: configStorage.dateOffset
            onValueChanged: configPage.configurationChanged()

            textFromValue: (value, locale) => i18ndp("plasma_calendar_alternatecalendar","%1 day", "%1 days", value)
            valueFromText: (text, locale) => parseInt(text)
        }
    }
}
