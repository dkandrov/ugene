/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "PythonSupport.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>



namespace U2 {


PythonSupport::PythonSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/python.png");
        grayIcon = QIcon(":external_tool_support/images/python_gray.png");
        warnIcon = QIcon(":external_tool_support/images/python_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="python.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="python2.7";
    #endif
#endif
    validMessage="Python ";
    validationArguments << "--version";
    
    description+=tr("Python scripts interpreter");
    versionRegExp=QRegExp("(\\d+.\\d+.\\d+)");
    toolKitName="python";

    muted = true;

    connect(this, SIGNAL(si_toolValidationStatusChanged(bool)), SLOT(sl_toolValidationStatusChanged(bool)));
}

void PythonSupport::sl_toolValidationStatusChanged(bool isValid) {
    Q_UNUSED(isValid);
    ScriptingTool::onPathChanged(this);
}


}//namespace
