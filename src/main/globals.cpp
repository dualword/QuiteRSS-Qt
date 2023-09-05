/* QuiteRSS-Qt (2023) http://github.com/dualword/QuiteRSS-Qt License:GNU GPL*/
/* ============================================================
* QuiteRSS is a open-source cross-platform RSS/Atom news feeds reader
* Copyright (C) 2011-2020 QuiteRSS Team <quiterssteam@gmail.com>
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
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
* ============================================================ */
#include "globals.h"

#ifdef HAVE_QT5
#include <QStandardPaths>
#include <QWebPage>
#else
#include <QDesktopServices>
#include <qwebkitversion.h>
#endif
#include <QCoreApplication>
#include <QDir>
#include <QStringBuilder>
#include <QRandomGenerator>
#include <QTextStream>

#include "settings.h"

Globals globals;

Globals::Globals()
  : logFileOutput_(true)
  , noDebugOutput_(true)
  , isInit_(false)
  , isPortable_(false)
  , resourcesDir_()
  , dataDir_()
  , cacheDir_()
  , soundNotifyDir_()
{

}

void Globals::init()
{
  // isPortable ...
#if defined(Q_OS_WIN)
  isPortable_ = true;
  QString fileName(QCoreApplication::applicationDirPath() + "/portable.dat");
  if (!QFile::exists(fileName)) {
    isPortable_ = false;
  }
#endif

  // Check Dir ...
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  resourcesDir_ = QCoreApplication::applicationDirPath();
#else
#if defined(Q_OS_MAC)
  resourcesDir_ = QCoreApplication::applicationDirPath() + "/../Resources";
#else
  resourcesDir_ = RESOURCES_DIR;
#endif
#endif

  if (isPortable_) {
    dataDir_ = QCoreApplication::applicationDirPath();
    cacheDir_ = "cache";
    soundNotifyDir_ = "sound";
  } else {
#ifdef HAVE_QT5
    dataDir_ = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    cacheDir_ = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#else
    dataDir_ = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    cacheDir_ = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
#endif
    soundNotifyDir_ = resourcesDir_ % "/sound";

    QDir dir(dataDir_);
    dir.mkpath(dataDir_);
  }

  QString path(dataDir_);
  // settings ...
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QString settingsFileName;
  if (isPortable_)
    settingsFileName = dataDir_ % "/" % QCoreApplication::applicationName() % ".ini";
  Settings::createSettings(settingsFileName);

  Settings settings;
  settings.beginGroup("Settings");
  noDebugOutput_ = settings.value("noDebugOutput", true).toBool();
  userAgent_ = settings.value("userAgent", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.120 Safari/537.36").toString();

  if(settings.value("rndUserAgent", false).toBool()) {
	QList<QString> arr;
	QFile file(path.append(QDir::separator()).append("user-agent.txt"));
	if (file.open(QFile::ReadOnly)) {
	  QTextStream stream(&file);
	  QString line;
      while (stream.readLineInto(&line)) {
    	  line = line.trimmed();
    	  if (line.startsWith("#") || line.length() <= 0) continue;
          arr.append(line);
      }
	}
	file.close();
	if(arr.size() > 0) userAgent_ = arr[QRandomGenerator::global()->bounded(arr.size())];
  }

  isInit_ = true;
}
