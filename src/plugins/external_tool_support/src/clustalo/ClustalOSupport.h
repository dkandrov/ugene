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

#ifndef _U2_CLUSTALO_SUPPORT_H_
#define _U2_CLUSTALO_SUPPORT_H_

#include <U2Core/ExternalToolRegistry.h>

#include <U2View/MSAEditor.h>

#include "utils/AlignMsaAction.h"

namespace U2 {

class ClustalOSupport : public ExternalTool {
    Q_OBJECT
public:
    ClustalOSupport();

    GObjectViewWindowContext *getViewContext() const {
        return viewCtx;
    }

    static const QString ET_CLUSTALO_ID;
    static const QString CLUSTALO_TMP_DIR;
public slots:
    void sl_runWithExtFileSpecify();

private:
    GObjectViewWindowContext *viewCtx;
};

class ClustalOSupportContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    ClustalOSupportContext(QObject *p);

protected slots:
    /** Re-aligns the current alignment opened in the view. */
    void sl_align();

    /** Initiates align-sequences-to-the-alignment UI & task. */
    void sl_addAlignmentToAlignment();

protected:
    void initViewContext(GObjectView *view) override;

private:
    /**
     * Checks that ClustalO tool setup is valid. Shows external tools setup dialog to user.
     * Sets error flag if ClustalO setup is not valid.
     */
    static void checkClustalOSetup(U2OpStatus &os);
};

}    // namespace U2
#endif    // _U2_CLUSTALO_SUPPORT_H_
