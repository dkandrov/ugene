/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GTWidget.h"
#include "GTMouseDriver.h"
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QWidget>
#include <QtGui/QMainWindow>

namespace U2 {

void GTWidget::click(U2OpStatus &os, QWidget *w, Qt::MouseButton mouseButton) {

    CHECK_SET_ERR(w != NULL, "GTWidget::click: widget is NULL");
    CHECK_SET_ERR(w->isVisible() == true, "GTWidget::click: widget is not visible");
    CHECK_SET_ERR(w->isEnabled() == true, "GTWidget::click: widget is not enabled");

    GTMouseDriver::moveTo(os, w->mapToGlobal(w->rect().center()));
    GTMouseDriver::click(os, mouseButton);
}

void GTWidget::setFocus(U2OpStatus &os, QWidget *w) {

    CHECK_SET_ERR(w != NULL, "GTWidget::setFocus: widget is NULL");

    GTWidget::click(os, w);
    GTGlobals::sleep(1000);

    CHECK_SET_ERR(w->hasFocus(), "Can't set focus on widget");
}

QWidget* GTWidget::findWidget(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, const FindOptions& options) {

    if (parentWidget == NULL) {
        parentWidget = AppContext::getMainWindow()->getQMainWindow();
    } 
    QWidget* widget = parentWidget->findChild<QWidget*>(widgetName);

    if (options.failIfNull) {
        CHECK_SET_ERR_RESULT(widget != NULL, "Widget " + widgetName + " not found", NULL);
    }

    return widget;
}

} //namespace
