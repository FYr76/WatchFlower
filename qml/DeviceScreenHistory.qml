/*!
 * This file is part of WatchFlower.
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2019
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

import QtQuick 2.9
import QtQuick.Layouts 1.2

import ThemeEngine 1.0

Item {
    id: deviceScreenHistory
    width: 400
    height: 300

    property string graphMode: settingsManager.graphHistory

    function updateHeader() {
        if (typeof myDevice === "undefined" || !myDevice) return
        //console.log("DeviceScreenHistory // updateHeader() >> " + myDevice)

        // Sensor battery level
        if (myDevice.hasBatteryLevel()) {
            imageBattery.visible = true
            imageBattery.color = Theme.colorIcon

            if (myDevice.deviceBattery > 95) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_full-24px.svg";
            } else if (myDevice.deviceBattery > 85) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_90-24px.svg";
            } else if (myDevice.deviceBattery > 75) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_80-24px.svg";
            } else if (myDevice.deviceBattery > 55) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_60-24px.svg";
            } else if (myDevice.deviceBattery > 45) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_50-24px.svg";
            } else if (myDevice.deviceBattery > 25) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_30-24px.svg";
            } else if (myDevice.deviceBattery > 15) {
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_20-24px.svg";
            } else if (myDevice.deviceBattery > 1) {
                if (myDevice.deviceBattery <= 10) imageBattery.color = Theme.colorYellow
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_10-24px.svg";
            } else {
                if (myDevice.deviceBattery === 0) imageBattery.color = Theme.colorRed
                imageBattery.source = "qrc:/assets/icons_material/baseline-battery_unknown-24px.svg";
            }
        } else {
            imageBattery.source = "qrc:/assets/icons_material/baseline-battery_unknown-24px.svg";
            imageBattery.visible = false
        }
    }

    function loadData() {
        if (typeof myDevice === "undefined" || !myDevice) return
/*
        console.log("DeviceScreenHistory // loadData() >> " + myDevice)

        console.log("hasHumiditySensor(): " + myDevice.hasHumiditySensor())
        console.log("hasSoilMoistureSensor(): " + myDevice.hasSoilMoistureSensor())
        console.log("hasTemperatureSensor(): " + myDevice.hasTemperatureSensor())
        console.log("hasLuminositySensor(): " + myDevice.hasLuminositySensor())
        console.log("hasConductivitySensor(): " + myDevice.hasConductivitySensor())

        console.log("hasData(hygro): " + myDevice.hasData("hygro"))
        console.log("hasData(temp): " + myDevice.hasData("temp"))
        console.log("hasData(luminosity): " + myDevice.hasData("luminosity"))
        console.log("hasData(conductivity): " + myDevice.hasData("conductivity"))
*/
        graphCount = 0

        if (myDevice.hasTemperatureSensor()) {
            tempGraph.visible = true
            tempGraph.loadGraph()
            graphCount += 1
        } else {
            tempGraph.visible = false
        }
        if (myDevice.hasHumiditySensor() || myDevice.hasSoilMoistureSensor()) {
            if (myDevice.deviceHumidity > 0 || myDevice.countData("hygro") > 0) {
                hygroGraph.visible = true
                hygroGraph.loadGraph()
                graphCount += 1
            } else {
                hygroGraph.visible = false
            }
        } else {
            hygroGraph.visible = false
        }
        if (myDevice.hasLuminositySensor()) {
            lumiGraph.visible = true
            lumiGraph.loadGraph()
            graphCount += 1
        } else {
            lumiGraph.visible = false
        }
        if (myDevice.hasConductivitySensor()) {
            if (myDevice.deviceConductivity > 0 || myDevice.countData("conductivity") > 0) {
                conduGraph.visible = true
                conduGraph.loadGraph()
                graphCount += 1
            } else {
                conduGraph.visible = false
            }
        } else {
            conduGraph.visible = false
        }

        updateSize()
        updateData()
    }

    function updateColors() {
        tempGraph.updateColors()
        hygroGraph.updateColors()
        lumiGraph.updateColors()
        conduGraph.updateColors()
    }

    function updateSize() {
        //console.log("width: " + graphGrid.width)
        //console.log("height: " + graphGrid.height)

        if (isMobile) {
            if (isPhone) {
                if (screenOrientation === Qt.PortraitOrientation) {
                    graphGrid.columns = 1
                    rectangleHeader.visible = true
                    rectangleHeader.height = 48
                } else {
                    graphGrid.columns = 2
                    rectangleHeader.visible = false
                    rectangleHeader.height = 0
                }
            }
            if (isTablet) {
                if (screenOrientation === Qt.PortraitOrientation || width < 480) {
                    graphGrid.columns = 1
                } else {
                    graphGrid.columns = 2
                }
            }
        } else {
            if (graphGrid.width < 1080) {
                graphGrid.columns = 1
            } else {
                graphGrid.columns = 2
            }
            if (graphGrid.width < 575) {
                buttonPanel.anchors.topMargin = 52
                buttonPanel.anchors.rightMargin = 0
                buttonPanel.anchors.right = undefined
                buttonPanel.anchors.horizontalCenter = rectangleHeader.horizontalCenter
                rectangleHeader.height = 96
            } else {
                buttonPanel.anchors.topMargin = 8
                buttonPanel.anchors.rightMargin = 8
                buttonPanel.anchors.horizontalCenter = undefined
                buttonPanel.anchors.right = rectangleHeader.right
                rectangleHeader.height = 48
            }
        }

        graphWidth = (graphGrid.width) / graphGrid.columns
        graphHeight = (graphGrid.height) / Math.ceil(graphCount / graphGrid.columns)

        if (graphCount === 3 && graphGrid.columns === 2) {
            if (myDevice.hasSoilMoistureSensor() && myDevice.hasData("hygro")) {
                hygroGraph.width = (graphWidth*2)
                lumiGraph.width = graphWidth
            } else if (myDevice.hasLuminositySensor() && myDevice.hasData("luminosity")) {
                hygroGraph.width = graphWidth
                lumiGraph.width = (graphWidth*2)
            }
        } else {
            hygroGraph.width = graphWidth
            lumiGraph.width = graphWidth
        }
    }

    function updateData() {
        if (typeof myDevice === "undefined" || !myDevice) return
        //console.log("ItemDeviceHistory // updateData() >> " + myDevice)

        if (myDevice.hasTemperatureSensor()) { tempGraph.updateGraph() }
        if (myDevice.hasHumiditySensor() || myDevice.hasSoilMoistureSensor()) { hygroGraph.updateGraph() }
        if (myDevice.hasLuminositySensor()) { lumiGraph.updateGraph() }
        if (myDevice.hasConductivitySensor()) { conduGraph.updateGraph() }
    }

    ////////////////////////////////////////////////////////////////////////////

    Rectangle {
        id: rectangleHeader
        color: Theme.colorForeground
        height: isMobile ? 48 : 96
        z: 5

        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0

        Row {
            id: buttonPanel
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16
            anchors.top: parent.top
            anchors.topMargin: isMobile ? 8 : 52

            ButtonWireframe {
                id: buttonDone
                width: 100
                height: 32

                fullColor: (graphMode === "monthly")
                secondaryColor: Theme.colorBackground

                text: qsTr("Month")
                onClicked: {
                    graphMode = "monthly"
                    updateData()
                }
            }

            ButtonWireframe {
                id: buttonDone1
                width: 100
                height: 32

                fullColor: (graphMode === "weekly")
                secondaryColor: Theme.colorBackground

                text: qsTr("Week")
                onClicked: {
                    graphMode = "weekly"
                    updateData()
                }
            }

            ButtonWireframe {
                id: buttonDone2
                width: 100
                height: 32

                fullColor: (graphMode === "daily")
                secondaryColor: Theme.colorBackground

                text: qsTr("Day")
                onClicked: {
                    graphMode = "daily"
                    updateData()
                }
            }
        }

        Text {
            id: textDeviceName
            height: 32
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 12

            visible: isDesktop

            text: myDevice.deviceName
            color: Theme.colorText
            font.pixelSize: Theme.fontSizeTitle
            font.capitalization: Font.AllUppercase
            verticalAlignment: Text.AlignVCenter

            ImageSvg {
                id: imageBattery
                width: 32
                height: 32
                rotation: 90
                anchors.verticalCenter: textDeviceName.verticalCenter
                anchors.left: textDeviceName.right
                anchors.leftMargin: 16

                source: "qrc:/assets/icons_material/baseline-battery_unknown-24px.svg"
                color: Theme.colorIcon
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    property int graphHeight: 256
    property int graphWidth: 256
    property int graphCount: 4

    Flow {
        id: graphGrid
        property var columns: 1

        anchors.top: rectangleHeader.bottom
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0

        onWidthChanged: updateSize()
        onHeightChanged: updateSize()

        ItemDataChart {
            id: hygroGraph
            height: graphHeight
            width: graphWidth
            graphDataSelected: "hygro"
            graphViewSelected: graphMode

            Text {
                id: hygroLegend
                anchors.left: parent.left
                anchors.leftMargin: 12
                text: myDevice.hasSoilMoistureSensor() ? qsTr("Moisture") : qsTr("Humidity")
                color: Theme.colorIcon
                font.bold: true
                font.pixelSize: 14
                font.capitalization: Font.AllUppercase
            }
        }

        ItemDataChart {
            id: tempGraph
            height: graphHeight
            width: graphWidth
            graphDataSelected: "temp"
            graphViewSelected: graphMode

            Text {
                id: tempLegend
                anchors.left: parent.left
                anchors.leftMargin: 12
                text: qsTr("Temperature")
                color: Theme.colorIcon
                font.bold: true
                font.pixelSize: 14
                font.capitalization: Font.AllUppercase
            }
        }

        ItemDataChart {
            id: lumiGraph
            height: graphHeight
            width: graphWidth
            graphDataSelected: "luminosity"
            graphViewSelected: graphMode

            Text {
                id: lumiLegend
                anchors.left: parent.left
                anchors.leftMargin: 12
                text: qsTr("Luminosity")
                color: Theme.colorIcon
                font.bold: true
                font.pixelSize: 14
                font.capitalization: Font.AllUppercase
            }
        }

        ItemDataChart {
            id: conduGraph
            height: graphHeight
            width: graphWidth
            graphDataSelected: "conductivity"
            graphViewSelected: graphMode

            Text {
                id: conduLegend
                anchors.left: parent.left
                anchors.leftMargin: 12
                text: qsTr("Fertility")
                color: Theme.colorIcon
                font.bold: true
                font.pixelSize: 14
                font.capitalization: Font.AllUppercase
            }
        }
    }
}
