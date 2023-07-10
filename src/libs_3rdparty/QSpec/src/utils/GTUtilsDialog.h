/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2023 UniPro <ugene@unipro.ru>
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

#pragma once
#include <core/CustomScenario.h>

#include <QDialogButtonBox>
#include <QStack>
#include <QTimer>

#include "GTGlobals.h"

namespace HI {

class HI_EXPORT Runnable {
public:
    virtual void run() = 0;
    virtual ~Runnable() = default;
};

class HI_EXPORT GUIDialogWaiter : public QObject {
    Q_OBJECT
public:
    /**
     * Dialog waiter activation time:
     * any test that expects a dialog waiter to be activated must 'sleep' at least this amount of time.
     */
    static constexpr int ACTIVATION_TIME = 100;

    enum class DialogType {
        Modal = 1,
        Popup = 2,
    };
    struct WaitSettings {
        WaitSettings(const QString& _objectName = "",
                     const DialogType& _dialogType = DialogType::Modal,
                     int _timeout = 20000,
                     bool _isRandomOrderWaiter = false,
                     const QString& _logName = "")
            : objectName(_objectName),
              dialogType(_dialogType),
              timeout(_timeout),
              isRandomOrderWaiter(_isRandomOrderWaiter),
              logName(_logName.isEmpty() ? _objectName : _logName) {
        }

        QString objectName;
        DialogType dialogType;
        int timeout;
        bool isRandomOrderWaiter = false;
        /** Logging name is used when objectName is empty (popup menus, messages). */
        QString logName;
    };

    GUIDialogWaiter(Runnable* _r, const WaitSettings& settings = WaitSettings());
    ~GUIDialogWaiter() override;

    const WaitSettings& getSettings() const {
        return settings;
    }

    /** Unique waiter identified. Autogenerated. */
    int waiterId = -1;

    Runnable* getRunnable() const {
        return runnable;
    }

public slots:
    void checkDialog();

private:
    Runnable* runnable = nullptr;
    WaitSettings settings;

    QTimer timer;
    int waitingTime = 0;
};

class HI_EXPORT Filler : public Runnable {
public:
    Filler(const GUIDialogWaiter::WaitSettings& settings, CustomScenario* scenario = nullptr);
    Filler(const QString& objectName, CustomScenario* scenario = nullptr);
    ~Filler() override;

    GUIDialogWaiter::WaitSettings getSettings() const;
    void run() override;

    /** Scenario to run. Empty by default. */
    virtual void commonScenario();

    /** Stack of the active fillers inside 'run()'. Can be used to improve logging. */
    static QStack<QString> activeFillerLogNamesStack;

    static QString generateFillerStackInfo();

protected:
    GUIDialogWaiter::WaitSettings settings;
    CustomScenario* scenario = nullptr;
};

class HI_EXPORT GTUtilsDialog {
    friend class GUIDialogWaiter;

public:
    static QDialogButtonBox* buttonBox(QWidget* dialog);

    static void clickButtonBox(QDialogButtonBox::StandardButton button);

    static void clickButtonBox(QWidget* dialog, QDialogButtonBox::StandardButton button);

    // if objectName is not empty, waits for QWidget with a given name
    static void waitForDialog(Runnable* r, const GUIDialogWaiter::WaitSettings& settings, bool isPrependToList = true);

    static void waitForDialog(Runnable* r, int timeout = 0, bool isRandomOrderWaiter = false, bool isPrependToList = true);

    /** Same as waitForDialog but adds waiter to the end of the current waiters list. */
    static void add(Runnable* r, const GUIDialogWaiter::WaitSettings& settings);

    /** Same as waitForDialog but adds waiter to the end of the current waiters list. */
    static void add(Runnable* r, int timeout = 0);

    /** Waits up to 'timeout' millis that all dialogs (runnables) are finished: the pool of GUIDialogWaiters is empty. */
    static void checkNoActiveWaiters(int timeoutMillis = 30000);

    static void removeRunnable(Runnable* runnable);

    /** Unconditionally deletes all GUIDialogWaiters. */
    static void cleanup();

private:
    static QList<GUIDialogWaiter*> waiterList;
};

}  // namespace HI
