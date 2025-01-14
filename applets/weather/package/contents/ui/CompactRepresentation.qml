/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.9

import QtQuick.Layouts 1.3

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.workspace.components 2.0 as WorkspaceComponents

Loader {
    id: compactRoot

    property var generalModel
    property var observationModel

    readonly property bool vertical: (Plasmoid.formFactor == PlasmaCore.Types.Vertical)
    readonly property bool showTemperature: Plasmoid.configuration.showTemperatureInCompactMode
    readonly property bool useBadge: Plasmoid.configuration.showTemperatureInBadge || Plasmoid.needsToBeSquare

    sourceComponent: (showTemperature && !useBadge) ? iconAndTextComponent : iconComponent
    Layout.fillWidth: compactRoot.vertical
    Layout.fillHeight: !compactRoot.vertical
    Layout.minimumWidth: item.Layout.minimumWidth
    Layout.minimumHeight: item.Layout.minimumHeight

    MouseArea {
        id: compactMouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.expanded = !root.expanded;
        }
    }

    Component {
        id: iconComponent

        Kirigami.Icon {
            readonly property int minIconSize: Math.max((compactRoot.vertical ? compactRoot.width : compactRoot.height), Kirigami.Units.iconSizes.small)

            source: Plasmoid.icon
            active: compactMouseArea.containsMouse
            // reset implicit size, so layout in free dimension does not stop at the default one
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
            Layout.minimumWidth: compactRoot.vertical ? Kirigami.Units.iconSizes.small : minIconSize
            Layout.minimumHeight: compactRoot.vertical ? minIconSize : Kirigami.Units.iconSizes.small

            WorkspaceComponents.BadgeOverlay {
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                visible: showTemperature && useBadge && text.length > 0

                text: observationModel.temperature
                icon: parent
            }
        }
    }

    Component {
        id: iconAndTextComponent

        IconAndTextItem {
            vertical: compactRoot.vertical
            iconSource: Plasmoid.icon
            active: compactMouseArea.containsMouse
            text: observationModel.temperature
        }
    }
}
