/**************************************************************************
 *   Copyright (C) 2006 by UC Davis Stahlberg Laboratory                   *
 *   HStahlberg@ucdavis.edu                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QDesktopServices>

#include "updateWindow.h"
#include "user_preferences.h"
#include "blockContainer.h"
#include <iostream>
using namespace std;

updateWindow::updateWindow(confData *conf, QWidget *parent)
: QWidget(parent, Qt::Window) {

    QAction *hideWindow = new QAction(tr("Close Window"), this);
    hideWindow->setShortcut(tr("Ctrl+W"));
    addAction(hideWindow);
    connect(hideWindow, SIGNAL(triggered()), this, SLOT(hide()));

    setWindowTitle("Software Update");

    data = conf;
    installedVersion = data->version_number();

    setMinimumWidth(600);
    //setFixedSize(QSize(568,410));

    QVBoxLayout *layout = new QVBoxLayout(this);

    updateTitle = new QLabel("2DX Version Information");
    QFont font = updateTitle->font();
    font.setBold(true);
    font.setPointSize(18);
    updateTitle->setFont(font);

    versionInfo = new QLabel();
    versionInfo->setWordWrap(true);
    setNormalPalette(versionInfo);

    revisionInfo = new QLabel();
    revisionInfo->setWordWrap(true);
    setNormalPalette(revisionInfo);

    QVBoxLayout* headerLayout = new QVBoxLayout;
    headerLayout->addWidget(updateTitle);
    headerLayout->addWidget(versionInfo);
    headerLayout->addWidget(revisionInfo);
    headerLayout->addStretch(1);

    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addLayout(headerLayout);
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topLayout->addWidget(spacer);

    if (data->getImage("appImage") != NULL) {
        QLabel *appImage = new QLabel();
        appImage->setPixmap(QPixmap::fromImage(*(data->getImage("appImage"))).scaledToHeight(100));
        appImage->setAlignment(Qt::AlignTop);
        topLayout->addWidget(appImage);
    } else {
        cerr << "appImage not found" << endl;
    }

    layout->addLayout(topLayout, 0);


    QFrame* hLine = new QFrame(this);
    hLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(hLine);

    updateText = new QTextBrowser(this);
    blockContainer* changesContainer = new blockContainer("Major release information", this);
    changesContainer->setMainWidget(updateText);
    layout->addWidget(changesContainer, 1);

    upgradeButton = new QPushButton("Update Version");
    connect(upgradeButton, SIGNAL(clicked()), this, SLOT(updateVersion()));
    layout->addWidget(upgradeButton, 0);

    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QUrl url = QUrl("http://www.2dx.unibas.ch/documentation/2dx-software/version-change-log/changes.htm");
    updateInf = manager->get(QNetworkRequest(url));
    connect(updateInf, SIGNAL(finished()), this, SLOT(updateTextBox()));

}

void updateWindow::updateTextBox() {
    if (updateInf->error() != QNetworkReply::NoError) {
        cout << "Http error: " << updateInf->errorString().toStdString() << endl;
        versionInfo->setText("No version information currently available.");
        updateText->insertPlainText("Http error: " + updateInf->errorString());
        return;
    }
    QByteArray updateHTML = updateInf->readAll();
    QStringList updateString = QString(updateHTML).split("######");
    QString currentFullVersion, currentReleaseText;
    
    if(updateString.size() > 1) currentFullVersion = updateString[1];
    if(updateString.size() > 2) currentReleaseText = updateString[2];
    
    QString currentVersion = currentFullVersion.split('-')[0];
    QString currentRevision;
    if (currentFullVersion.split('-').size() > 1) currentRevision = currentFullVersion.split('-')[1];
    QString remindUpdate = UserPreferences(data).getRemindUpdate().toLower();
    QString intVersion = currentVersion, intInstalled = installedVersion;
    intVersion.remove('.');
    intInstalled.remove('.');
    revisionInfo->setText("");
    upgradeButton->hide();
    if (intVersion.toInt() > intInstalled.toInt()) {
        versionInfo->setText("Version " + currentVersion + " is now available, your current version is " + installedVersion + ".<br>Would you like to upgrade?<br>");
        setWarningPalette(versionInfo);
        upgradeButton->show();
        if (remindUpdate != currentVersion) {
            int choice = QMessageBox::question(this, tr("New version available"), tr("A new version of 2dx is available. <br> Would you like to upgrade?"), tr("Tell Me More"), tr("Skip this version"), tr("Not now"));
            if (choice == 0) show();
            if (choice == 1) UserPreferences(data).setRemindUpdate(currentVersion);
        }
    } else {
        versionInfo->setText("The currently installed version, " + installedVersion + " is the latest available.");
        setNormalPalette(versionInfo);

        if (!currentRevision.isEmpty()) {
            if (currentRevision != data->version_revision()) {
                revisionInfo->setText("Revision " + currentRevision + " is now available, your current revision is " + data->version_revision() + ".<br>Would you like to upgrade?<br>");
                setWarningPalette(revisionInfo);
                upgradeButton->show();
            } else {
                revisionInfo->setText("The currently installed revision, " + data->version_revision() + " is the latest available.");
                setNormalPalette(revisionInfo);
            }
        } else {
            revisionInfo->setText("No revision information available");
            setNormalPalette(revisionInfo);
        }
    }

    currentReleaseText.insert(0, "<html><body>");
    currentReleaseText.append("</body></html>");
    updateText->insertHtml(currentReleaseText);
    updateText->moveCursor(QTextCursor::Start);
}

void updateWindow::updateVersion() {
    QDesktopServices::openUrl(QUrl("http://2dx.org/download/"));
}

void updateWindow::setWarningPalette(QWidget* widget) {
    QPalette pal = widget->palette();
    pal.setColor(QPalette::WindowText, Qt::red);
    widget->setPalette(pal);
}

void updateWindow::setNormalPalette(QWidget* widget) {
    QPalette pal = widget->palette();
    pal.setColor(QPalette::WindowText, Qt::black);
    widget->setPalette(pal);
}