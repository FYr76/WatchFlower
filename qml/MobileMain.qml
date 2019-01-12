
import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.0

import StatusBar 0.1

ApplicationWindow {
    id: applicationWindow
    color: "#E0FAE7"
    visible: true

    minimumWidth: 400
    minimumHeight: 640

    flags: Qt.Window | Qt.MaximizeUsingFullscreenGeometryHint

    Material.theme: Material.System
    Material.accent: Material.Green

    StatusBar {
        theme: Material.System
        color: Material.color(Material.Green, Material.Shade500)
    }

    Drawer {
        id: drawer
        width: 0.80 * applicationWindow.width
        height: applicationWindow.height

        onOpenedChanged: drawerscreen.updateDrawerFocus()
        MobileDrawer { id: drawerscreen }
    }

    // Events handling /////////////////////////////////////////////////////////

    Connections {
        target: header
        onLeftMenuClicked: {
            drawer.open()
        }
    }
    Connections {
        target: systrayManager
        onSettingsClicked: {
            content.state = "Settings"
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.BackButton | Qt.ForwardButton
        onClicked: {
            if (mouse.button === Qt.BackButton) {
                content.state = "DeviceList"
            } else if (mouse.button === Qt.ForwardButton) {
                if (curentlySelectedDevice)
                    content.state = "DeviceDetails"
            }
        }
    }
    Shortcut {
        sequence: StandardKey.Back
        onActivated: {
            content.state = "DeviceList"
        }
    }
    Shortcut {
        sequence: StandardKey.Forward
        onActivated: {
            if (curentlySelectedDevice)
                content.state = "DeviceDetails"
        }
    }
    Item {
        focus: true
        Keys.onBackPressed: {
            if (Qt.platform.os === "android" || Qt.platform.os === "ios") {
                if (content.state === "DeviceList") {
                    // hide windows?
                } else {
                    content.state = "DeviceList"
                }
            } else {
                content.state = "DeviceList"
            }
        }
    }
    onClosing: {
        if (Qt.platform.os === "android" || Qt.platform.os === "ios") {
            close.accepted = false;
        } else {
            close.accepted = false;
            applicationWindow.hide()
        }
    }

    // QML /////////////////////////////////////////////////////////////////////

    property var curentlySelectedDevice

    MobileHeader {
        id: header
        anchors.top: parent.top
    }

    Rectangle {
        id: content
        color: "#e0fae7"
        anchors.top: header.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        DeviceList {
            anchors.fill: parent
            id: screenDeviceList
        }
        DeviceScreen {
            anchors.fill: parent
            id: screenDeviceDetails
        }
        Settings {
            anchors.fill: parent
            id: screenSettings
        }

        onStateChanged: {
            drawerscreen.updateDrawerFocus()
        }

        state: "DeviceList"
        states: [
            State {
                name: "DeviceList"

                PropertyChanges {
                    target: screenDeviceList
                    visible: true
                }
                PropertyChanges {
                    target: screenDeviceDetails
                    visible: false
                }
                PropertyChanges {
                    target: screenSettings
                    visible: false
                }
            },
            State {
                name: "DeviceDetails"

                PropertyChanges {
                    target: screenDeviceList
                    visible: false
                }
                PropertyChanges {
                    target: screenDeviceDetails
                    visible: true
                }
                PropertyChanges {
                    target: screenSettings
                    visible: false
                }
                StateChangeScript {
                    name: "secondScript"
                    script: screenDeviceDetails.loadDevice()
                }
            },
            State {
                name: "Settings"

                PropertyChanges {
                    target: screenDeviceList
                    visible: false
                }
                PropertyChanges {
                    target: screenDeviceDetails
                    myDevice: curentlySelectedDevice
                    visible: false
                }
                PropertyChanges {
                    target: screenSettings
                    visible: true
                }
            }
        ]
    }
}