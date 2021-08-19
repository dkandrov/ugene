/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2021 UniPro <ugene@unipro.ru>
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

#include "DeviationGraph.h"

#include <U2Algorithm/RollingArray.h>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>

#include "DNAGraphPackPlugin.h"

/* TRANSLATOR U2::DeviationGraphFactory */

namespace U2 {
static QString nameByType(DeviationGraphFactory::GDeviationType t) {
    if (t == DeviationGraphFactory::AT) {
        return DeviationGraphFactory::tr("AT Deviation (A-T)/(A+T)");
    }
    return DeviationGraphFactory::tr("GC Deviation (G-C)/(G+C)");
}

DeviationGraphFactory::DeviationGraphFactory(GDeviationType t, QObject *p)
    : GSequenceGraphFactory(nameByType(t), p) {
    if (t == DeviationGraphFactory::AT) {
        devPair.first = 'A';
        devPair.second = 'T';
    } else {
        devPair.first = 'G';
        devPair.second = 'C';
    }
}

bool DeviationGraphFactory::isEnabled(const U2SequenceObject *o) const {
    const DNAAlphabet *al = o->getAlphabet();
    if (al->isRNA() && (devPair.first == 'T' || devPair.second == 'T')) {
        return false;
    }
    return al->isNucleic();
}

QList<QSharedPointer<GSequenceGraphData>> DeviationGraphFactory::createGraphs(GSequenceGraphView *view) {
    assert(isEnabled(view->getSequenceObject()));
    return {QSharedPointer<GSequenceGraphData>(new GSequenceGraphData(view, getGraphName(), new DeviationGraphAlgorithm(devPair)))};
}

//////////////////////////////////////////////////////////////////////////
// DeviationGraphAlgorithm

DeviationGraphAlgorithm::DeviationGraphAlgorithm(const QPair<char, char> &_p)
    : p(_p) {
}
void DeviationGraphAlgorithm::windowStrategyWithoutMemorize(QVector<float> &res, const QByteArray &seqArr, qint64 startPos, qint64 window, qint64 step, qint64 nSteps, U2OpStatus &os) {
    assert(startPos >= 0);
    const char *seq = seqArr.constData();

    for (int i = 0; i < nSteps; i++) {
        int start = startPos + i * step;
        int end = start + window;
        assert(end <= seqArr.size());
        int first = 0;
        int second = 0;
        for (int x = start; x < end; x++) {
            CHECK_OP(os, );
            char c = seq[x];
            if (c == p.first) {
                first++;
                continue;
            }
            if (c == p.second) {
                second++;
            }
        }
        res.append((first - second) / qMax(0.001f, (float)(first + second)));
    }
}
QPair<int, int> DeviationGraphAlgorithm::matchOnStep(const QByteArray &seqArr, int begin, int end) {
    const char *seq = seqArr.constData();
    assert(begin >= 0 && end <= seqArr.size());

    QPair<int, int> res(0, 0);
    for (int j = begin; j < end; ++j) {
        char c = seq[j];
        if (c == p.first) {
            res.first++;
            continue;
        }
        if (c == p.second) {
            res.second++;
        }
    }
    return res;
}
void DeviationGraphAlgorithm::sequenceStrategyWithMemorize(QVector<float> &res, const QByteArray &seq, const U2Region &vr, qint64 window, qint64 step, U2OpStatus &os) {
    int rsize = window / step;
    RollingArray<int> raF(rsize);
    RollingArray<int> raS(rsize);
    int endPos = vr.endPos();
    int globalCountF = 0;
    int globalCountS = 0;
    int nextI = 0;
    int firstValue = vr.startPos + window - step;
    for (int i = vr.startPos; i < endPos; i = nextI) {
        CHECK_OP(os, );
        nextI = i + step;
        QPair<int, int> result = matchOnStep(seq, i, nextI);
        globalCountF += result.first;
        globalCountS += result.second;
        raF.push_back_pop_front(result.first);
        raS.push_back_pop_front(result.second);
        if (i >= firstValue) {
            int vF = raF.get(0);
            int vS = raS.get(0);
            res.append((globalCountF - globalCountS) / qMax(0.001f, (float)(globalCountF + globalCountS)));
            globalCountF -= vF;
            globalCountS -= vS;
        }
    }
}

void DeviationGraphAlgorithm::calculate(QVector<float> &res, U2SequenceObject *o, qint64 window, qint64 step, U2OpStatus &os) {
    U2Region vr(0, o->getSequenceLength());
    int nSteps = GSequenceGraphUtils::getNumSteps(vr, window, step);
    res.reserve(nSteps);

    QByteArray seq = o->getWholeSequenceData(os);
    CHECK_OP(os, );

    int startPos = vr.startPos;
    if (window % step != 0) {
        windowStrategyWithoutMemorize(res, seq, startPos, window, step, nSteps, os);
    } else {
        sequenceStrategyWithMemorize(res, seq, vr, window, step, os);
    }
}

}  // namespace U2
