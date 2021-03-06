/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

import QtQuick 2.4
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import TimeLog 1.0

Page {
    id: dialog

    function close() {
        TimeTracker.backRequested()
    }

    title: qsTranslate("settings", "Sync timeout")
    visible: false

    header: ToolBarMaterial {
        title: dialog.title

        onLeftActivated: dialog.close()
    }

    Flickable {
        anchors.bottomMargin: Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        anchors.fill: parent
        contentWidth: settingsItem.width
        contentHeight: settingsItem.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar { }

        Item {
            id: settingsItem

            width: dialog.width
            implicitHeight: container.height + container.anchors.margins * 2

            Column {
                id: container

                anchors.margins: 16
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 16

                SpinBox {
                    anchors.horizontalCenter: parent.horizontalCenter
                    to: 24 * 3600
                    editable: true
                    value: AppSettings.syncCacheTimeout
                    onValueChanged: AppSettings.syncCacheTimeout = value
                }

                Label {
                    width: parent.width
                    color: Material.hintTextColor
                    wrapMode: Text.Wrap
                    text: qsTranslate("settings", "With this setting you can control, how much time"
                                      + " (in seconds) application would wait after last record"
                                      + " before the sync.\n\n"
                                      + "To disable sync by timeout, set it to \u20180\u2019.")
                }
            }
        }
    }
}
