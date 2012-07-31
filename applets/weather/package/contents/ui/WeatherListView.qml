/*
* Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
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

import QtQuick 1.1
import "Utils.js" as Utils

Column {
    id: root

    property alias rows: repeater.model
    property Component delegate
    property bool roundedRows: true

    Repeater {
        id: repeater

        Rectangle {
            id: rect
            anchors.left: parent.left
            anchors.right: parent.right
            height: 18
            radius: root.roundedRows ? 5 : 0
            color: Utils.setAlphaF(theme.textColor, ((index+1)/repeater.count)*0.3);

            Loader {
                anchors.fill: parent
                sourceComponent: root.delegate
                onLoaded: item.rowIndex = index;
            }
        }
    }
}
