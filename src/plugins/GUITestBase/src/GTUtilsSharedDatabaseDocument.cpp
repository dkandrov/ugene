/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2022 UniPro <ugene@unipro.ru>
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

#include "GTUtilsSharedDatabaseDocument.h"
#include <drivers/GTMouseDriver.h>
#include <primitives/GTWidget.h>
#include <utils/GTThread.h>

#include <QDateTime>
#include <QTreeView>

#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectViewModel.h>

#include "GTDatabaseConfig.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "primitives/GTMenu.h"
#include "primitives/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AddFolderDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportToDatabaseDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/SharedConnectionsDialogFiller.h"
#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsSharedDatabaseDocument"

static QString getSuiteFolderPrefix() {
    // Suites runs in parallel, each creates folders with a unique prefix.
    QString suiteNumber = qgetenv("UGENE_GUI_TEST_SUITE_NUMBER");
    if (suiteNumber.isEmpty()) {
        suiteNumber = "0";
    }
    return "_tmp" + suiteNumber + "_";
}

#define GT_METHOD_NAME "removeTempContentFromOtherTests"

/**
 * Removes temprorary content generated by other tests in the same suite.
 * This helps to limit unstable drag & drop + scroll behavior when project tree is too large.
 */
static void removeTempContentFromOtherTests(HI::GUITestOpStatus &os, Document *document) {
    QModelIndexList documentItems = GTUtilsProjectTreeView::findIndeciesInProjectViewNoWait(os, document->getName(), QModelIndex(), 0, GTGlobals::FindOptions(false));
    if (documentItems.isEmpty()) {
        return;
    }

    QModelIndex documentItem = documentItems[0];
    QString tmpFolderPrefix = getSuiteFolderPrefix();
    int maxToRemove = 4;  // if there are too many documents to remove the test may fail by timeout.
    QTreeView *treeView = GTUtilsProjectTreeView::getTreeView(os);
    for (int item = 0; item < maxToRemove; item++) {
        bool isRemoved = false;
        for (int i = 0; i < treeView->model()->rowCount(documentItem); i++) {
            QModelIndex itemIndex = treeView->model()->index(i, 0, documentItem);
            QString itemName = itemIndex.data(Qt::DisplayRole).toString();
            if (itemName.startsWith(tmpFolderPrefix)) {
                GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Remove selected items", GTGlobals::UseMouse));
                GTUtilsProjectTreeView::callContextMenu(os, itemIndex);
                GTThread::waitForMainThread();
                GTUtilsDialog::waitAllFinished(os);
                isRemoved = true;
                break;
            }
        }
        if (!isRemoved) {
            break;
        }
    }
}

#undef GT_METHOD_NAME

#define GT_METHOD_NAME "connectToTestDatabase"
Document *GTUtilsSharedDatabaseDocument::connectToTestDatabase(HI::GUITestOpStatus &os, bool isRemoveTempContent) {
    GTLogTracer lt;
    QString conName = "ugene_gui_test";
    GTDatabaseConfig::initTestConnectionInfo(conName);

    QList<SharedConnectionsDialogFiller::Action> actions;
    actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CLICK, conName);
    actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CONNECT, conName);
    GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions));

    GTMenu::clickMainMenuItem(os, QStringList() << "File"
                                                << "Connect to UGENE shared database...");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTThread::waitForMainThread();
    GTUtilsDialog::waitAllFinished(os);

    Document *document = GTUtilsSharedDatabaseDocument::getDatabaseDocumentByName(os, conName);
    if (isRemoveTempContent) {
        removeTempContentFromOtherTests(os, document);
    }

    CHECK_SET_ERR_RESULT(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString(), nullptr);
    return document;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "connectToUgenePublicDatabase"
Document *GTUtilsSharedDatabaseDocument::connectToUgenePublicDatabase(HI::GUITestOpStatus &os) {
    GTLogTracer lt;
    QString conName = "UGENE public database";
    {
        QList<SharedConnectionsDialogFiller::Action> actions;
        actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CLICK, conName);
        actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CONNECT, conName);
        GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions));
    }
    GTMenu::clickMainMenuItem(os, QStringList() << "File"
                                                << "Connect to UGENE shared database...");

    CHECK_SET_ERR_RESULT(!lt.hasErrors(), "Errors in log: " + lt.getJoinedErrorString(), nullptr);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDialog::waitAllFinished(os);
    return GTUtilsSharedDatabaseDocument::getDatabaseDocumentByName(os, conName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getDatabaseDocumentByName"
Document *GTUtilsSharedDatabaseDocument::getDatabaseDocumentByName(HI::GUITestOpStatus &os, const QString &name) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(!name.isEmpty(), "Name is empty", nullptr);

    const QModelIndex databaseDocIndex = GTUtilsProjectTreeView::findIndex(os, name);
    GT_CHECK_RESULT(databaseDocIndex.isValid(), QString("Can't find the document with name '%1'").arg(name), nullptr);

    return ProjectViewModel::toDocument(databaseDocIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "disconnectDatabase"
void GTUtilsSharedDatabaseDocument::disconnectDatabase(HI::GUITestOpStatus &os, Document *databaseDoc) {
    GT_CHECK(databaseDoc != nullptr, "databaseDoc is NULL");
    disconnectDatabase(os, databaseDoc->getName());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "disconnectDatabase"
void GTUtilsSharedDatabaseDocument::disconnectDatabase(HI::GUITestOpStatus &os, const QString &name) {
    GTUtilsDocument::removeDocument(os, name);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "genTestFolderName"
QString GTUtilsSharedDatabaseDocument::genTestFolderName(const QString &baseName) {
    return getSuiteFolderPrefix() + baseName + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemIndex"
QModelIndex GTUtilsSharedDatabaseDocument::getItemIndex(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &itemPath, bool mustExist) {
    GT_CHECK_RESULT(databaseDoc != nullptr, "databaseDoc is NULL", QModelIndex());
    GT_CHECK_RESULT(!itemPath.isEmpty(), "Folder path is empty", QModelIndex());

    QStringList folders = itemPath.split(U2ObjectDbi::PATH_SEP, QString::SkipEmptyParts);

    GTGlobals::FindOptions options;
    options.depth = 1;
    options.failIfNotFound = mustExist;

    QModelIndex itemIndex;
    for (int time = 0; time < GT_OP_WAIT_MILLIS && !itemIndex.isValid(); time += GT_OP_CHECK_MILLIS) {
        GTGlobals::sleep(time > 0 ? GT_OP_CHECK_MILLIS : 0);
        itemIndex = GTUtilsProjectTreeView::findIndex(os, QStringList() << databaseDoc->getName());
        foreach (const QString &folder, folders) {
            itemIndex = GTUtilsProjectTreeView::findIndex(os, folder, itemIndex, options);
            if (!itemIndex.isValid()) {
                break;
            }
        }
        if (!mustExist) {
            break;
        }
    }
    GT_CHECK_RESULT(itemIndex.isValid() || !mustExist, "Item not found: " + itemPath, itemIndex);
    return itemIndex;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createFolder"
void GTUtilsSharedDatabaseDocument::createFolder(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &parentFolderPath, const QString &newFolderName) {
    Q_UNUSED(os);
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!parentFolderPath.isEmpty(), "Parent folder path is empty");
    GT_CHECK(!newFolderName.isEmpty(), "New folder's name is empty");
    GT_CHECK(!newFolderName.contains(U2ObjectDbi::PATH_SEP), "New folder's name contains invalid characters");

    QModelIndex parentFolderIndex = getItemIndex(os, databaseDoc, parentFolderPath);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__ADD_MENU << ACTION_PROJECT__CREATE_FOLDER));
    GTUtilsDialog::waitForDialog(os, new AddFolderDialogFiller(os, newFolderName, GTGlobals::UseMouse));

    GTMouseDriver::moveTo(GTUtilsProjectTreeView::getItemCenter(os, parentFolderIndex));
    GTMouseDriver::click(Qt::RightButton);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createPath"
void GTUtilsSharedDatabaseDocument::createPath(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &path) {
    Q_UNUSED(os);
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(path.startsWith(U2ObjectDbi::ROOT_FOLDER), "Path is not in the canonical form");

    QString parentFolder = U2ObjectDbi::ROOT_FOLDER;
    const QStringList folders = path.split(U2ObjectDbi::PATH_SEP, QString::SkipEmptyParts);
    foreach (const QString &folder, folders) {
        bool alreadyExist = getItemIndex(os, databaseDoc, parentFolder + U2ObjectDbi::PATH_SEP + folder, false).isValid();
        if (!alreadyExist) {
            createFolder(os, databaseDoc, parentFolder, folder);
        }
        parentFolder += U2ObjectDbi::PATH_SEP + folder;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemPath"
QString GTUtilsSharedDatabaseDocument::getItemPath(HI::GUITestOpStatus &os, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(itemIndex.isValid(), "Item index is invalid", QString());

    ProjectViewModel::Type itemType = ProjectViewModel::itemType(itemIndex);
    switch (itemType) {
        case ProjectViewModel::DOCUMENT:
            return U2ObjectDbi::ROOT_FOLDER;

        case ProjectViewModel::FOLDER: {
            Folder *folder = ProjectViewModel::toFolder(itemIndex);
            GT_CHECK_RESULT(nullptr != folder, "Can't convert item to folder", QString());
            return folder->getFolderPath();
        }

        case ProjectViewModel::OBJECT: {
            QString folderPath;
            const QModelIndex parentItemIndex = itemIndex.parent();
            GT_CHECK_RESULT(parentItemIndex.isValid(), "Parent item index of the object item is invalid", QString());

            ProjectViewModel::Type parentItemType = ProjectViewModel::itemType(parentItemIndex);
            if (ProjectViewModel::DOCUMENT == parentItemType) {
                folderPath = U2ObjectDbi::ROOT_FOLDER;
            } else if (ProjectViewModel::FOLDER == parentItemType) {
                Folder *folder = ProjectViewModel::toFolder(parentItemIndex);
                GT_CHECK_RESULT(nullptr != folder, "Can't convert parent item to folder", QString());
                folderPath = folder->getFolderPath();
            } else {
                GT_CHECK_RESULT(false, "Can't recognize the parent item", QString());
            }

            GObject *object = ProjectViewModel::toObject(itemIndex);
            GT_CHECK_RESULT(nullptr != object, "Can't convert item to object", QString());
            return folderPath + U2ObjectDbi::PATH_SEP + object->getGObjectName();
        }

        default:
            GT_CHECK_RESULT(false, "Can't recognize the item", QString());
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expantToItem"
void GTUtilsSharedDatabaseDocument::expantToItem(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &itemPath) {
    GT_CHECK(databaseDoc != nullptr, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    QStringList folders = itemPath.split(U2ObjectDbi::PATH_SEP, QString::SkipEmptyParts);
    GTGlobals::FindOptions findOptions;
    findOptions.depth = 1;
    QModelIndex databaseDocIndex = GTUtilsProjectTreeView::findIndex(os, databaseDoc->getName(), findOptions);

    QTreeView *treeView = GTUtilsProjectTreeView::getTreeView(os);
    GTUtilsProjectTreeView::scrollToIndexAndMakeExpanded(os, treeView, databaseDocIndex);

    CHECK(!folders.isEmpty(), );
    folders.pop_back();

    QModelIndex prevFolderIndex = databaseDocIndex;
    foreach (const QString &folder, folders) {
        QModelIndex folderIndex = GTUtilsProjectTreeView::findIndex(os, folder, prevFolderIndex, findOptions);
        GTUtilsProjectTreeView::doubleClickItem(os, folderIndex);
        prevFolderIndex = folderIndex;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expantToItem"
void GTUtilsSharedDatabaseDocument::expantToItem(HI::GUITestOpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    GT_CHECK(databaseDoc != nullptr, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    QString itemPath = getItemPath(os, itemIndex);
    expantToItem(os, databaseDoc, itemPath);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doubleClickItem"
void GTUtilsSharedDatabaseDocument::doubleClickItem(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &itemPath) {
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    expantToItem(os, databaseDoc, itemPath);
    const QModelIndex itemIndex = getItemIndex(os, databaseDoc, itemPath);
    GTUtilsProjectTreeView::doubleClickItem(os, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doubleClickItem"
void GTUtilsSharedDatabaseDocument::doubleClickItem(HI::GUITestOpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    expantToItem(os, databaseDoc, itemIndex);
    GTUtilsProjectTreeView::doubleClickItem(os, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openView"
void GTUtilsSharedDatabaseDocument::openView(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &itemPath) {
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    doubleClickItem(os, databaseDoc, itemPath);
    GTGlobals::sleep(100);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openView"
void GTUtilsSharedDatabaseDocument::openView(HI::GUITestOpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    doubleClickItem(os, databaseDoc, itemIndex);
    GTGlobals::sleep(100);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callImportDialog"
void GTUtilsSharedDatabaseDocument::callImportDialog(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    QModelIndex itemIndex = getItemIndex(os, databaseDoc, itemPath);
    callImportDialog(os, databaseDoc, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callImportDialog"
void GTUtilsSharedDatabaseDocument::callImportDialog(HI::GUITestOpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    GT_CHECK(databaseDoc != nullptr, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    expantToItem(os, databaseDoc, itemIndex);
    GTMouseDriver::moveTo(GTUtilsProjectTreeView::getItemCenter(os, itemIndex));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__ADD_MENU << ACTION_PROJECT__IMPORT_TO_DATABASE, GTGlobals::UseMouse));
    GTMouseDriver::click(Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItemExists"
void GTUtilsSharedDatabaseDocument::checkItemExists(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &itemPath) {
    getItemIndex(os, databaseDoc, itemPath);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItemsExist"
void GTUtilsSharedDatabaseDocument::checkItemsExist(HI::GUITestOpStatus &os, Document *databaseDoc, const QStringList &itemsPaths) {
    foreach (const QString &itemPath, itemsPaths) {
        checkItemExists(os, databaseDoc, itemPath);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkThereAraNoItemsExceptListed"
void GTUtilsSharedDatabaseDocument::checkThereAreNoItemsExceptListed(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &parentPath, const QStringList &itemsPaths) {
    checkItemsExist(os, databaseDoc, itemsPaths);

    QModelIndex parentIndex = getItemIndex(os, databaseDoc, parentPath);
    QModelIndexList subIndecies = GTUtilsProjectTreeView::findIndeciesInProjectViewNoWait(os, "", parentIndex);
    GT_CHECK(subIndecies.size() == itemsPaths.size(), QString("Parent item contains %1 subitems, expected % subitems").arg(subIndecies.size()).arg(itemsPaths.size()));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "importFiles"
void GTUtilsSharedDatabaseDocument::importFiles(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &dstFolderPath, const QStringList &filesPaths, const QVariantMap &options) {
    Q_UNUSED(os);
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!filesPaths.isEmpty(), "Files paths are not provided");

    QList<ImportToDatabaseDialogFiller::Action> actions;

    QVariantMap addFileAction;
    addFileAction.insert(ImportToDatabaseDialogFiller::Action::ACTION_DATA__PATHS_LIST, filesPaths);
    actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::ADD_FILES, addFileAction);

    if (!options.isEmpty()) {
        actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::EDIT_GENERAL_OPTIONS, options);
    }

    actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::IMPORT, QVariantMap());

    GTUtilsDialog::waitForDialog(os, new ImportToDatabaseDialogFiller(os, actions));

    createPath(os, databaseDoc, dstFolderPath);
    GTGlobals::sleep();
    callImportDialog(os, databaseDoc, dstFolderPath);

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(15000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "importDirs"
void GTUtilsSharedDatabaseDocument::importDirs(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &dstFolderPath, const QStringList &dirsPaths, const QVariantMap &options) {
    Q_UNUSED(os);
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!dirsPaths.isEmpty(), "Dirs paths are not provided");

    QList<ImportToDatabaseDialogFiller::Action> actions;

    QVariantMap addDirAction;
    addDirAction.insert(ImportToDatabaseDialogFiller::Action::ACTION_DATA__PATHS_LIST, dirsPaths);
    actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::ADD_DIRS, addDirAction);

    if (!options.isEmpty()) {
        actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::EDIT_GENERAL_OPTIONS, options);
    }

    actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::IMPORT, QVariantMap());

    GTUtilsDialog::waitForDialog(os, new ImportToDatabaseDialogFiller(os, actions));

    createPath(os, databaseDoc, dstFolderPath);
    callImportDialog(os, databaseDoc, dstFolderPath);

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(15000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "importProjectItems"
void GTUtilsSharedDatabaseDocument::importProjectItems(HI::GUITestOpStatus &os, Document *databaseDoc, const QString &dstFolderPath, const QMap<QString, QStringList> &projectItems, const QVariantMap &options) {
    GT_CHECK(nullptr != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!projectItems.isEmpty(), "Project items are not provided");

    QList<ImportToDatabaseDialogFiller::Action> actions;

    QVariantMap addProjectItemAction;
    addProjectItemAction.insert(ImportToDatabaseDialogFiller::Action::ACTION_DATA__PROJECT_ITEMS_LIST, convertProjectItemsPaths(projectItems));
    actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::ADD_PROJECT_ITEMS, addProjectItemAction);

    if (!options.isEmpty()) {
        actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::EDIT_GENERAL_OPTIONS, options);
    }

    actions << ImportToDatabaseDialogFiller::Action(ImportToDatabaseDialogFiller::Action::IMPORT, QVariantMap());

    GTUtilsDialog::waitForDialog(os, new ImportToDatabaseDialogFiller(os, actions));

    createPath(os, databaseDoc, dstFolderPath);
    callImportDialog(os, databaseDoc, dstFolderPath);

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(15000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "convertProjectItemsPaths"
const QVariant GTUtilsSharedDatabaseDocument::convertProjectItemsPaths(const QMap<QString, QStringList> &projectItems) {
    QMap<QString, QVariant> result;
    foreach (const QString &documentName, projectItems.keys()) {
        result.insert(documentName, projectItems[documentName]);
    }
    return result;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}  // namespace U2
