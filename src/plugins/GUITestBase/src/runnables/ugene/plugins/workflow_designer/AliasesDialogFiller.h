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

#ifndef ALIASESDIALOGFILLER_H
#define ALIASESDIALOGFILLER_H

#include "utils/GTUtilsDialog.h"

namespace U2 {
using namespace HI;

class AliasesDialogFiller : public Filler {
public:
    AliasesDialogFiller(HI::GUITestOpStatus &_os, QMap<QPoint *, QString> _map)
        : Filler(_os, "SchemaAliasesConfigurationDialog"),
          map(_map) {
    }
    void commonScenario();

private:
    QMap<QPoint *, QString> map;
};
}  // namespace U2
#endif  // ALIASESDIALOGFILLER_H
