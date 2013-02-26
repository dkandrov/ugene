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
#include "AddTreeDialog.h"
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorDataList.h>


namespace U2 {

AddTreeDialog::AddTreeDialog(QWidget* p, MSAEditor* _msa)
: QDialog(p), msa(_msa){
    setupUi(this);
    buildTreeCheck->setChecked(true);
}

void AddTreeDialog::accept() {
    if(buildTreeCheck->isChecked()) {
        msa->getTreeManager()->buildTreeWithDialog();
    }
    else {
        msa->getTreeManager()->addTreeToMSA();
    }
    hide();
}

}//namespace

