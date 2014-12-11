/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Gui/MainWindow.h>

#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorOverviewArea.h>
#include <U2View/MSAGraphOverview.h>

#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionPanelMSA.h"
#include "api/GTMouseDriver.h"
#include "api/GTMSAEditorStatusWidget.h"
#include "api/GTToolbar.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsMsaEditor"

#define GT_METHOD_NAME "getGraphOverviewTopLeftPixelColor"
QColor GTUtilsMsaEditor::getGraphOverviewPixelColor(U2OpStatus &os, const QPoint &point) {
    return GTWidget::getColor(os, getGraphOverview(os), point);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getGraphOverview"
MSAGraphOverview *GTUtilsMsaEditor::getGraphOverview(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MSAGraphOverview *result = GTWidget::findExactWidget<MSAGraphOverview *>(os, MSAEditorOverviewArea::OVERVIEW_AREA_OBJECT_NAME + QString("_graph"), activeWindow);
    GT_CHECK_RESULT(NULL != result, "MSAGraphOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getNameListArea"
MSAEditorNameList *GTUtilsMsaEditor::getNameListArea(U2OpStatus &os) {
    QWidget *activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_OP(os, NULL);

    MSAEditorNameList *result = GTWidget::findExactWidget<MSAEditorNameList *>(os, "msa_editor_name_list", activeWindow);
    GT_CHECK_RESULT(NULL != result, "MSAGraphOverview is not found", NULL);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequenceNameRect"
QRect GTUtilsMsaEditor::getSequenceNameRect(U2OpStatus &os, const QString &sequenceName) {
    Q_UNUSED(os);
    MSAEditorNameList *nameList = getNameListArea(os);
    GT_CHECK_RESULT(NULL != nameList, "MSAEditorNameList not found", QRect());

    const int rowHeight = GTUtilsMSAEditorSequenceArea::getRowHeight(os);
    const QStringList names = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
    const int rowNumber = names.indexOf(sequenceName);
    GT_CHECK_RESULT(0 <= rowNumber, QString("Sequence '%1' not found").arg(sequenceName), QRect());

    return QRect(nameList->mapToGlobal(QPoint(0, rowHeight * rowNumber)), nameList->mapToGlobal(QPoint(nameList->width(), rowHeight * (rowNumber + 1))));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickSequenceName"
void GTUtilsMsaEditor::clickSequenceName(U2OpStatus &os, const QString &sequenceName) {
    Q_UNUSED(os);

    const QRect sequenceNameRect = getSequenceNameRect(os, sequenceName);
    GTMouseDriver::moveTo(os, sequenceNameRect.center());
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getReferenceSequenceName"
QString GTUtilsMsaEditor::getReferenceSequenceName(U2OpStatus &os) {
    return GTUtilsOptionPanelMsa::getReference(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleCollapsingMode"
void GTUtilsMsaEditor::toggleCollapsingMode(U2OpStatus &os) {
    Q_UNUSED(os);
    GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Switch on/off collapsing"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleCollapsingMode"
void GTUtilsMsaEditor::toggleCollapsingGroup(U2OpStatus &os, const QString &groupName) {
    Q_UNUSED(os);

    const QRect sequenceNameRect = getSequenceNameRect(os, groupName);

    const QPoint magicExpandButtonOffset(15, 5);
    GTMouseDriver::moveTo(os, sequenceNameRect.topLeft() + magicExpandButtonOffset);
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequencesCount"
int GTUtilsMsaEditor::getSequencesCount(U2OpStatus &os) {
    QWidget *statusWidget = GTWidget::findWidget(os, "msa_editor_status_bar");
    return GTMSAEditorStatusWidget::getSequencesCount(os, statusWidget);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
