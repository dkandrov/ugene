/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_HIGHLIGHTNING_TAB_H_
#define _U2_MSA_HIGHLIGHTNING_TAB_H_

#include <QtGui/QtGui>
#include <U2View/MSAEditor.h>

namespace U2 {

class U2VIEW_EXPORT MSAHighlightingTab : public QWidget
{
    Q_OBJECT
public:
    MSAHighlightingTab(MSAEditor* msa);

private slots:
    void sl_sync();
    void sl_updateHint(const QString &);

private:
    QWidget* createColorGroup();
    QWidget* createHighlightingGroup();
    void initColorCB();

    MSAEditor *msa;
    MSAEditorSequenceArea *seqArea;
    QComboBox *colorScheme;
    QComboBox *highlightingScheme;
    QLabel *hint;
    QCheckBox *useDots;
};

} // namespace

#endif