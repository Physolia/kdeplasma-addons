/*
    SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQml 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.plasma.core 2.0 as PlasmaCore

QtObject {
    id: manager

    property QQC2.ScrollView target

    property real x: 0.0
    property real y: 0.0

    /*
        In onSave handler implementors should persist values x and y provided
        by this manager object.
    */
    signal save()

    /*
        In onRestore handler implementors should populate values x and y of
        this manager object with whatever was previously persisted during
        onSave() signal handling.
    */
    signal restore()

    function forceSave() {
        if (target !== null) {
            x = target.QQC2.ScrollBar.horizontal.position;
            y = target.QQC2.ScrollBar.vertical.position;
            save();
        }
    }

    function forceRestore() {
        if (target !== null) {
            restore();
            target.QQC2.ScrollBar.horizontal.position = x;
            target.QQC2.ScrollBar.vertical.position = y;
        }
    }

    // Save scrolling position when it changes, but throttle to avoid
    // killing a storage disk.
    property list<QtObject> data: [
        Timer {
            id: throttle
            interval: PlasmaCore.Units.humanMoment
            repeat: false
            running: false
            onTriggered: manager.forceSave()
        },

        Connections {
            target: manager.target ? manager.target.QQC2.ScrollBar.horizontal : null
            function onPositionChanged() {
                throttle.restart();
            }
        },

        Connections {
            target: manager.target ? manager.target.QQC2.ScrollBar.vertical : null
            function onPositionChanged() {
                throttle.restart();
            }
        }
    ]

    Component.onCompleted: {
        // Give it some time to lay out the text, because at this
        // point in time content size is not reliable yet.
        Qt.callLater(forceRestore);
    }

    Component.onDestruction: forceSave()
}
