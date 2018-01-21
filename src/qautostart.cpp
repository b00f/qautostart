// The MIT License (MIT)
//
// Copyright (C) 2016 Mostafa Sedaghat Joo (mostafa.sedaghat@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "qautostart.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QFile>
#include <QDir>


Autostart::Autostart() {

}

#if defined (Q_OS_WIN)
#define REG_KEY "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

bool Autostart::isAutostart() const {
    QSettings settings(REG_KEY, QSettings::NativeFormat);

    if (settings.value(appName()).isNull()) {
        return false;
    }

    return true;
}

void Autostart::setAutostart(bool autostart) {
    QSettings settings(REG_KEY, QSettings::NativeFormat);

    if (autostart) {
        settings.setValue(appName() , appPath().replace('/','\\'));
    } else {
        settings.remove(appPath());
    }
}

QString Autostart::appPath() const {
    return QCoreApplication::applicationFilePath() + " --autostart";
}

#elif defined (Q_OS_MAC)

bool Autostart::isAutostart() const {
    QStringList args;
    args << "-e tell application \"System Events\" contains login item \"" + appPath() + "\""; ///???

    return QProcess::execute("osascript", args);
}

void Autostart::setAutostart(bool autostart) {
    // Remove any existing login entry for this app first, in case there was one
    // from a previous installation, that may be under a different launch path.
    {
        QStringList args;
        args << "-e tell application \"System Events\" to delete login item \"" + appPath() + "\"";

        QProcess::execute("osascript", args);
    }

    // Now install the login item, if needed.
    if ( autostart )
    {
        QStringList args;
        args << "-e tell application \"System Events\" to make login item at end with properties {path:\"" + appPath() + "\", hidden:false}";

        QProcess::execute("osascript", args);
    }
}

QString Autostart::appPath() const {
    QDir appDir = QDir(qApp->applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    QString absolutePath = dir.absolutePath();
    // absolutePath will contain a "/" at the end,
    // but we want the clean path to the .app bundle
    if ( absolutePath.length() > 0 && absolutePath.right(1) == "/" ) {
        absolutePath.chop(1);
    }

    return absolutePath + " --autostart";
}

#elif defined (Q_OS_LINUX)
bool Autostart::isAutostart() const {
    QFileInfo check_file(QDir::homePath() + "/.config/autostart/" + appName() +".desktop");

    if (check_file.exists() && check_file.isFile()) {
        return true;
    }

    return false;
}

void Autostart::setAutostart(bool autostart) {
    QString path = QDir::homePath() + "/.config/autostart/";
    QString name = appName() +".desktop";
    QFile file(path+name);

    file.remove();

    if(autostart) {
        QDir dir(path);
        if(!dir.exists()) {
            dir.mkpath(path);
        }

        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << "[Desktop Entry]" << endl;
            stream << "Exec=" << appPath() << endl;
            stream << "Type=Application" << endl;
        }
    }
}

QString Autostart::appPath() const {
    return QCoreApplication::applicationFilePath() + " --autostart";
}

#else

bool Autostart::isAutostart() const {
    return false;
}

void Autostart::setAutostart(bool autostart) {
    Q_UNUSED(autostart);
}

QString Autostart::appPath() const {
    return QString();
}
#endif

QString Autostart::appName() const {
    return QFileInfo(QCoreApplication::applicationFilePath()).fileName();
}
