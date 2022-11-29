/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.colorpicker 2.0 as ColorPicker
import "logic.js" as Logic

DropArea {
    id: dropArea

    property alias color: colorCircle.color
    property alias colorCircle: colorCircle

    property bool containsAcceptableDrag: false

    onEntered: containsAcceptableDrag = (drag.hasColor || drag.hasUrls || ColorPicker.Utils.isValidColor(drag.text))
    onExited: containsAcceptableDrag = false
    onDropped: {
        if (drop.hasColor) {
            addColorToHistory(drop.colorData)
        } else if (ColorPicker.Utils.isValidColor(drop.text)) {
            addColorToHistory(drop.text)
        } else if (drop.hasUrls) {
            const component = Qt.createComponent("ImageColors.qml");
            drop.urls.forEach(path => {
                component.incubateObject(dropArea, {
                    "source": path,
                }, Qt.Asynchronous);
            });
            component.destroy();
        }
        containsAcceptableDrag = false
    }

    PlasmaComponents3.ToolButton {
        id: colorButton

        anchors.fill: parent

        // indicate viable drag...
        checked: dropArea.containsAcceptableDrag || colorButton.pressed || dragHandler.active
        checkable: checked
        display: PlasmaComponents3.AbstractButton.IconOnly
        text: Logic.formatColor(colorCircle.color, root.defaultFormat)

        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction
        Drag.mimeData: {
            "application/x-color": colorCircle.color,
            "text/plain": colorButton.text
        }

        onClicked: Plasmoid.expanded = !Plasmoid.expanded

        DragHandler {
            id: dragHandler

            onActiveChanged: if (active) {
                colorCircle.grabToImage((result) => {
                    colorButton.Drag.imageSource = result.url;
                    colorButton.Drag.active = dragHandler.active;
                });
            } else {
                colorButton.Drag.active = false;
                colorButton.Drag.imageSource = "";
            }
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: Plasmoid.title
            subText: colorButton.text
        }

        Rectangle {
            id: colorCircle
            anchors.centerIn: parent
            // try to match the color-picker icon in size
            width: PlasmaCore.Units.roundToIconSize(pickerIcon.width) * 0.75
            height: PlasmaCore.Units.roundToIconSize(pickerIcon.height) * 0.75
            radius: width / 2

            function luminance(color) {
                if (!color) {
                    return 0;
                }

                // formula for luminance according to https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef

                const a = [color.r, color.g, color.b].map(v =>
                    (v <= 0.03928) ? v / 12.92 : Math.pow(((v + 0.055) / 1.055), 2.4 ));

                return a[0] * 0.2126 + a[1] * 0.7152 + a[2] * 0.0722;
            }

            border {
                color: PlasmaCore.Theme.textColor
                width: {
                    const contrast = luminance(PlasmaCore.Theme.viewBackgroundColor) / luminance(colorCircle.color) + 0.05;

                    // show border only if there's too little contrast to the surrounding view or color is transparent
                    if (contrast > 3 && colorCircle.color.a > 0.5) {
                        return 0;
                    } else {
                        return Math.round(Math.max(PlasmaCore.Units.devicePixelRatio, width / 20));
                    }
                }
            }
        }
    }
}

