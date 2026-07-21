
/* Goal Line Custom QML Type.

   Copyright (C) 2023 Saksham Attri.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import goal.model 1.0

RowLayout {

    property string toolTipText: (resNumID.color === Qt.color(
                                      "green")) ? "Goal was met at line "
                                                  + line : ((resNumID.color === Qt.color(
                                                                 "blue")) ? "Goal was met at line " + line + "\n\t but the proof has errors" : ((resNumID.color === Qt.color("red")) ? "Goal was not met" : "Not yet evaluated")) //aaaaaaaaaaaaaaaaaaaaaaaaaaa

    spacing: 10
    width: (parent) ? parent.width : 0
    Layout.fillWidth: true

    Label {
        id: goalLineNumID

        height: goalTextID
        width: height + 10

        ToolTip.visible: toolTipText ? moID.containsMouse : false
        ToolTip.text: toolTipText

        MouseArea {
            id: moID
            anchors.fill: parent
            hoverEnabled: true
            onClicked: console.log(line)
        }

        Text {
            id: resNumID
            anchors.centerIn: parent
            font.italic: true
            text: (line > 0) ? line : ((line === -3) ? "X" : "?")
            color: (text === "?") ? (darkMode ? "yellow" : "brown") : ((text === "X") ? "red" : ((model.valid) ? "green" : "blue"))
        }
    }

    TextField {
        id: goalTextID

        height: font.pointSize + 10
        width: 200
        Layout.fillWidth: true
        background: Rectangle {
            color: darkMode ? "#332940" : "lightgrey"
        }

        text: model.text
        //        wrapMode: TextArea.Wrap
        //        placeholderText: qsTr("Start Typing here...")

        // Implementing Keyboard Macros
        onTextChanged: {
            let replaced = false;

            if (goalTextID.length >= 2) {
                const last_two = text.slice(cursorPosition - 2, cursorPosition)
                if (last_two.includes('/\\')) {
                    goalTextID.remove(cursorPosition - 2, cursorPosition)
                    goalTextID.insert(cursorPosition, "\u2227")
                    replaced = true;
                } else if (last_two.includes('\\/')) {
                    goalTextID.remove(cursorPosition - 2, cursorPosition)
                    goalTextID.insert(cursorPosition, "\u2228")
                    replaced = true;
                } else if (last_two.includes('->')) {
                    goalTextID.remove(cursorPosition - 2, cursorPosition)
                    goalTextID.insert(cursorPosition, "\u2192")
                    replaced = true;
                } else if (last_two.includes('<' + "\u2192")) {
                    goalTextID.remove(cursorPosition - 2, cursorPosition)
                    goalTextID.insert(cursorPosition, "\u2194")
                    replaced = true;
                }
            }

            if (!replaced && goalTextID.length >= 1) {
                const last_one = text.slice(cursorPosition - 1, cursorPosition)
                let replacement = "";
                switch (last_one) {
                    case '^': replacement = "\u2295"; break; // XOR
                    case '&': replacement = "\u2227"; break; // AND
                    case '|': replacement = "\u2228"; break; // OR
                    case '~': replacement = "\u00AC"; break; // NOT
                    case '$': replacement = "\u2192"; break; // CON
                    case '%': replacement = "\u2194"; break; // BIC
                    case '@': replacement = "\u2200"; break; // UNV
                    case '#': replacement = "\u2203"; break; // EXL
                    case '!': replacement = "\u22A4"; break; // TAU
                    case '?': replacement = "\u22A5"; break; // CTR
                    case ':': replacement = "\u2208"; break; // ELM
                    case '>': replacement = "\u2349"; break; // NIL
                }
                if (replacement !== "") {
                    goalTextID.remove(cursorPosition - 1, cursorPosition)
                    goalTextID.insert(cursorPosition, replacement)
                }
            }
        }

        onTextEdited: fileModified = true

        onEditingFinished: {
            if (model.text !== text) fileModified = true
            model.text = text
        }
    }

    Button {
        id: goalPlusID

        height: goalTextID.height

        onClicked: goalOptionsID.open()

        Text {
            anchors.centerIn: parent
            text: "+ / \u2013"
            color: darkMode ? "white" : "black"
        }

        Menu {
            id: goalOptionsID

            Action {
                text: "Add Goal"
                onTriggered: {
                    theGoals.insertgLine(index + 1, -2, false, "")
                    fileModified = true
                }
            }

            Action {
                text: "Remove Goal"
                onTriggered: {
                    if (goalDataID.rowCount() > 1) {
                        theGoals.removegLineAt(index)
                        fileModified = true
                    } else {
                        console.log("Invalid Operation: Cannot remove all Lines")
                        cConnector.evalText = "⚠ " + qsTr("Invalid Operation: At least one goal line must remain.")
                    }
                }
            }
        }
    }
}
