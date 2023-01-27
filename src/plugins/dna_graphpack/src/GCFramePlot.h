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

#include <QAction>
#include <QBitArray>
#include <QList>

#include <U2View/GSequenceGraphView.h>

#include "DNAGraphPackPlugin.h"

namespace U2 {

class AnnotatedDNAView;

class GCFramePlotFactory : public GSequenceGraphFactory {
    Q_OBJECT
public:
    GCFramePlotFactory(QObject* p);
    QList<QSharedPointer<GSequenceGraphData>> createGraphs(GSequenceGraphView* v);
    GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v);
    bool isEnabled(const U2SequenceObject* o) const;
};

class GCFramePlotAlgorithm : public GSequenceGraphAlgorithm {
public:
    GCFramePlotAlgorithm(int offset);

    void calculate(QVector<float>& result, U2SequenceObject* sequenceObject, qint64 window, qint64 step, U2OpStatus& os) override;

private:
    void windowStrategyWithoutMemorize(QVector<float>& res, const QByteArray& seq, int startPos, qint64 window, qint64 step, qint64 nSteps, U2OpStatus& os);

    int offset;
};

}  // namespace U2
