#!/bin/bash

VERSION=$1

VERSION_MAJOR=$(cut -d'.' -f-2 <<< $VERSION)
VERSION_MINOR=$(cut -d'.' -f3 <<< $VERSION)
VERSION_COMPRESSED=$(sed 's/\.//g' <<< "$VERSION")

FILE="qt-opensource-linux-x64-${VERSION}.run"

wget "http://download.qt.io/official_releases/qt/${VERSION_MAJOR}/${VERSION}/${FILE}"

cat > install.qs << EOF

function Controller() {
    installer.autoRejectMessageBoxes();
    installer.installationFinished.connect(function() {
        gui.clickButton(buttons.NextButton,3000);
    })
}

Controller.prototype.WelcomePageCallback = function() {
    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.CredentialsPageCallback = function() {
    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.IntroductionPageCallback = function() {
    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    gui.currentPageWidget().TargetDirectoryLineEdit.setText(installer.value("InstallerDirPath") + "/Qt");
    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    var widget = gui.currentPageWidget();

    widget.deselectAll();

    widget.selectComponent("qt.qt5.$VERSION_COMPRESSED.gcc_64");
    widget.selectComponent("qt.qt5.$VERSION_COMPRESSED.qtwebengine");

    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.LicenseAgreementPageCallback = function() {
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton,10000);
}

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    gui.clickButton(buttons.NextButton,3000);
}

Controller.prototype.FinishedPageCallback = function() {
var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm
if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
    checkBoxForm.launchQtCreatorCheckBox.checked = false;
}
    gui.clickButton(buttons.FinishButton);
}

EOF

chmod +x ${FILE}
./${FILE} --script install.qs --platform minimal --verbose

echo "export CMAKE_PREFIX_PATH=${PWD}/Qt/${VERSION}/gcc_64/lib/cmake/"
