/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
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

#include "primitives/GTComboBox.h"
#include <utils/GTThread.h>

#include <QListView>
#include <QStandardItemModel>

#include "drivers/GTKeyboardDriver.h"
#include "drivers/GTMouseDriver.h"
#include "primitives/GTWidget.h"

namespace HI {

#define GT_CLASS_NAME "GTComboBox"

#define GT_METHOD_NAME "setCurrentIndex"
void GTComboBox::setCurrentIndex(GUITestOpStatus &os, QComboBox *comboBox, int index, bool checkVal, GTGlobals::UseMethod method) {
    GT_CHECK(comboBox != nullptr, "QComboBox* == NULL");

    // Access to the internal comboBox->view() must be done from the main thread (the view is lazily instantiated).
    class MainThreadAction : public CustomScenario {
    public:
        MainThreadAction(QComboBox *comboBox, int index, GTGlobals::UseMethod method)
            : CustomScenario(), comboBox(comboBox), index(index), method(method) {
        }
        void run(HI::GUITestOpStatus &os) {
            if (comboBox->currentIndex() == index) {
                return;
            }
            int comboCount = comboBox->count();
            GT_CHECK(index >= 0 && index < comboCount, "invalid index");

            if (comboBox->isEditable()) {
                GTWidget::click(os, comboBox, Qt::LeftButton, QPoint(comboBox->rect().width() - 10, 10));
                GTThread::waitForMainThread();
            } else if (!comboBox->view()->isVisible()) {    // activate dropdown if it is not visible.
                GTWidget::click(os, comboBox);
                GTThread::waitForMainThread();
            }

            switch (method) {
            case GTGlobals::UseKey:
            case GTGlobals::UseKeyBoard: {
                int currIndex = comboBox->currentIndex() == -1 ? 0 : comboBox->currentIndex();
                Qt::Key directionKey = index > currIndex ? Qt::Key_Down : Qt::Key_Up;

                int pressCount = qAbs(index - currIndex);

                for (int i = 0; i < pressCount; i++) {
                    GTKeyboardDriver::keyClick(directionKey);
                    GTThread::waitForMainThread();
                }
                break;
            }
            case GTGlobals::UseMouse: {
                QListView *listView = comboBox->findChild<QListView *>();
                GT_CHECK(listView != nullptr, "list view not found");
                QModelIndex modelIndex = listView->model()->index(index, 0);
                GTWidget::scrollToIndex(os, listView, modelIndex);
                QPoint itemPointLocal = listView->visualRect(modelIndex).center();
                QPoint itemPointGlobal = listView->viewport()->mapToGlobal(itemPointLocal);
                qDebug("GT_DEBUG_MESSAGE moving to the list item: %d %d -> %d %d", itemPointLocal.x(), itemPointLocal.y(), itemPointGlobal.x(), itemPointGlobal.y());
                GTMouseDriver::moveTo(itemPointGlobal);
                break;
            }
            default:
                GT_CHECK(false, "Unexpected method");
            }
        }
        QComboBox *comboBox;
        int index;
        GTGlobals::UseMethod method;
    };
    GTThread::runInMainThread(os, new MainThreadAction(comboBox, index, method));
    GTThread::waitForMainThread();

    // Activate the final action from a separate thread. Reason: it may trigger other popups or dialogs (they can't be processed in the main thread).
    if (method == GTGlobals::UseMouse) {
        GTMouseDriver::click();
    } else {
        GTKeyboardDriver::keyClick(Qt::Key_Enter);
    }

    if (checkVal) {
        int currIndex = comboBox->currentIndex();
        for (int time = 0; time < GT_OP_WAIT_MILLIS && currIndex != index; time += GT_OP_CHECK_MILLIS) {
            currIndex = comboBox->currentIndex();
        }
        GT_CHECK(currIndex == index, "Can't set index! Current: " + QString::number(currIndex) + ", expected: " + QString::number(index));
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCurrentText"
void GTComboBox::setCurrentTextWithKeyboard(GUITestOpStatus &os, QComboBox *comboBox, const QString &text, bool checkVal) {
    GT_CHECK(comboBox != NULL, "QComboBox* == NULL");

    if (comboBox->currentText() == text) {
        return;
    }

    int index = comboBox->findText(text, Qt::MatchExactly);
    GT_CHECK(index != -1, "Text " + text + " was not found");

    if (comboBox->isEditable()) {
        QPoint p(comboBox->rect().width() - 10, 10);
        GTWidget::click(os, comboBox, Qt::LeftButton, p);
    } else {
        GTWidget::setFocus(os, comboBox);
    }
    GTGlobals::sleep();

    for (auto c : text) {
        GTKeyboardDriver::keyClick(c.toLatin1(), Qt::NoModifier, false);
        GTGlobals::sleep(200);
    }

    GTKeyboardDriver::keyClick(Qt::Key_Enter);

    GTThread::waitForMainThread();
    GTGlobals::sleep(500);

    if (checkVal) {
        QString currentText = comboBox->currentText();
        GT_CHECK(currentText == text, "Can't set text in setCurrentText: " + text);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setIndexWithText"
void GTComboBox::setIndexWithText(GUITestOpStatus &os, QComboBox *const comboBox, const QString &text, bool checkVal, GTGlobals::UseMethod method) {
    GT_CHECK(comboBox != nullptr, "QComboBox* == NULL");

    int index = comboBox->findText(text, Qt::MatchExactly);
    GT_CHECK(index != -1, "Text " + text + " was not found");

    switch (method) {
    case GTGlobals::UseKeyBoard:
        setCurrentTextWithKeyboard(os, comboBox, text, checkVal);
        break;
    default:
        setCurrentIndex(os, comboBox, index, checkVal, method);
        break;
    }

    GTThread::waitForMainThread();
    if (checkVal) {
        QString currentText = comboBox->currentText();
        GT_CHECK(currentText == text, "Can't set text in setIndexWithText: " + text + ", method: " + QString::number(method));
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setIndexWithText"
void GTComboBox::setIndexWithText(GUITestOpStatus &os, const QString &comboBoxName, QWidget const *const parent, const QString &text, bool checkVal, GTGlobals::UseMethod method) {
    setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, comboBoxName, parent), text, checkVal, method);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getCurrentText"
QString GTComboBox::getCurrentText(GUITestOpStatus &os, QComboBox *const comboBox) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(comboBox != NULL, "comboBox is NULL", "");
    return comboBox->currentText();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getCurrentText"
QString GTComboBox::getCurrentText(GUITestOpStatus &os, const QString &comboBoxName, const QWidget *const parent) {
    return getCurrentText(os, GTWidget::findExactWidget<QComboBox *>(os, comboBoxName, parent));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getValues"
QStringList GTComboBox::getValues(GUITestOpStatus &os, QComboBox *comboBox) {
    QStringList result;
    GT_CHECK_RESULT(NULL != comboBox, "Combobox is NULL", result);
    for (int i = 0; i < comboBox->count(); i++) {
        result << comboBox->itemText(i);
    }
    return result;
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkValues"
void GTComboBox::checkValues(GUITestOpStatus &os, QComboBox *comboBox, const QStringList &values) {
    GT_CHECK(NULL != comboBox, "comboBox is NULL");

    GTWidget::setFocus(os, comboBox);
    GTGlobals::sleep();

    QListView *view = comboBox->findChild<QListView *>();
    GT_CHECK(NULL != view, "list view is not found");
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(view->model());
    GT_CHECK(NULL != model, "model is not found");
    QList<QStandardItem *> items = model->findItems("", Qt::MatchContains);

    for (QStandardItem *item : items) {
        if (values.contains(item->data().toString())) {
            if (item->checkState() != Qt::Checked) {
                QModelIndex modelIndex = item->index();
                view->scrollTo(modelIndex);
                GTGlobals::sleep(500);
                QRect itemRect = view->visualRect(modelIndex);
                QPoint checkPoint(itemRect.left() + 10, itemRect.center().y());
                GTMouseDriver::moveTo(view->viewport()->mapToGlobal(checkPoint));
                GTMouseDriver::click();
                GTGlobals::sleep(500);
                GT_CHECK(item->checkState() == Qt::Checked, "Item is not checked: " + item->data().toString());
            }
        } else {
            if (item->checkState() == Qt::Checked) {
                QModelIndex modelIndex = item->index();
                view->scrollTo(modelIndex);
                GTGlobals::sleep(500);
                QRect itemRect = view->visualRect(modelIndex);
                QPoint checkPoint(itemRect.left() + 10, itemRect.center().y());
                GTMouseDriver::moveTo(view->viewport()->mapToGlobal(checkPoint));
                GTMouseDriver::click();
                GTGlobals::sleep(500);
                GT_CHECK(item->checkState() != Qt::Checked, "Item is checked: " + item->data().toString());
            }
        }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkValuesPresence"
void GTComboBox::checkValuesPresence(GUITestOpStatus &os, QComboBox *comboBox, const QStringList &values) {
    Q_UNUSED(os)
    GT_CHECK(NULL != comboBox, "ComboBox is NULL");

    foreach (const QString &s, values) {
        int index = comboBox->findText(s);
        GT_CHECK(index != -1, "text not found " + s);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkCurrentValue"
void GTComboBox::checkCurrentValue(GUITestOpStatus &os, QComboBox *comboBox, const QString &expectedValue) {
    Q_UNUSED(os);
    GT_CHECK(comboBox != nullptr, "ComboBox is NULL");
    GT_CHECK(expectedValue == comboBox->currentText(), QString("An unexpected value: expect '%1', got '%2'").arg(expectedValue).arg(comboBox->currentText()));
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}    // namespace HI
