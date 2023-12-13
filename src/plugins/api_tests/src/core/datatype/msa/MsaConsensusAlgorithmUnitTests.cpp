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

#include "MsaConsensusAlgorithmUnitTests.h"

#include <QSharedPointer>

#include <U2Algorithm/BuiltInAssemblyConsensusAlgorithms.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Core/U2AlphabetUtils.h>

namespace U2 {

static char c1(const QString& column, int threshold) {
    auto bestAlphabet = U2AlphabetUtils::findBestAlphabet(column.toLocal8Bit());
    MultipleSequenceAlignment alignment("Test alignment name", bestAlphabet);

    for (int i = 0; i < column.size(); i++) {
        alignment->addRow(QString::number(i), column.mid(i, 1).toLocal8Bit());
    }

    auto factory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInAssemblyConsensusAlgorithms::LEVITSKY_ALGO);
    QSharedPointer<MSAConsensusAlgorithm> algorithm(factory->createAlgorithm(alignment, false, nullptr));
    algorithm->setThreshold(threshold);

    return algorithm->getConsensusChar(alignment, 0, {});
}

static char cN(const QStringList& alignmentRows, int threshold) {
    auto bestAlphabet = U2AlphabetUtils::findBestAlphabet(alignmentRows.join("").toLocal8Bit());
    MultipleSequenceAlignment alignment("Test alignment name", bestAlphabet);

    for (int i = 0; i < alignmentRows.size(); i++) {
        const auto& row = alignmentRows[i];
        alignment->addRow(QString::number(i), row.toLocal8Bit());
    }

    auto factory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInAssemblyConsensusAlgorithms::LEVITSKY_ALGO);
    QSharedPointer<MSAConsensusAlgorithm> algorithm(factory->createAlgorithm(alignment, false, nullptr));
    algorithm->setThreshold(threshold);

    return algorithm->getConsensusChar(alignment, 0, {});
}

/*
Code table from Victor Levitsky:
A       A       1
T       T       1
G       G       1
C       C       1
W       A,T     2       WEAK
R       A,G     2       PURINE
M       A,C     2       AMINO- (+ charge)
K       T,G     2       KETO- (- charge)
Y       T,C     2       PYRIMIDINE
S       G,C     2       STRONG
B       T,G,C   3       not A (B is near with A in Latin alphabet)
V       A,G,C   3       not T (---||---)
H       A,T,C   3       not G (---||---)
D       A,T,G   3       not C (---||---)
N       A,T,G,C 4       Any

*/

IMPLEMENT_TEST(MsaConsensusAlgorithmUnitTests, levitskyCheckColumnBase) {
#define CHECK_LEVITSKY(c, f) CHECK_EQUAL((c), (f), __LINE__);
    // 1. One base mix
    //  Single base in the column - this base in consensus
    CHECK_LEVITSKY('A', c1("A", 50));
    CHECK_LEVITSKY('C', c1("C", 50));
    CHECK_LEVITSKY('G', c1("G", 50));
    CHECK_LEVITSKY('T', c1("T", 50));
    CHECK_LEVITSKY('U', c1("U", 50));
    CHECK_LEVITSKY('N', c1("N", 50));
    CHECK_LEVITSKY('-', c1("-", 50));

    CHECK_LEVITSKY('A', c1("AA", 50));
    CHECK_LEVITSKY('C', c1("CC", 50));
    CHECK_LEVITSKY('G', c1("GG", 50));
    CHECK_LEVITSKY('T', c1("TT", 50));
    CHECK_LEVITSKY('U', c1("UU", 50));
    CHECK_LEVITSKY('N', c1("NN", 50));
    CHECK_LEVITSKY('-', c1("--", 50));

    CHECK_LEVITSKY('A', c1("A-", 50));
    CHECK_LEVITSKY('C', c1("C-", 50));
    CHECK_LEVITSKY('G', c1("G-", 50));
    CHECK_LEVITSKY('T', c1("T-", 50));
    CHECK_LEVITSKY('U', c1("U-", 50));
    CHECK_LEVITSKY('N', c1("N-", 50));

    CHECK_LEVITSKY('-', c1("A--", 50));
    CHECK_LEVITSKY('-', c1("C--", 50));
    CHECK_LEVITSKY('-', c1("G--", 50));
    CHECK_LEVITSKY('-', c1("T--", 50));
    CHECK_LEVITSKY('-', c1("U--", 50));
    CHECK_LEVITSKY('-', c1("N--", 50));

    // 2. Two bases mix
    // 2.1 The case, when consensus character is defined only by bases of the column
    //
    // A is not taken into account, because its share is 33%, which is less than 50%
    CHECK_LEVITSKY('C', c1("ACC", 50));
    // A and C are not taken into account, because their share is 33% and 66%, which is less than 70%.
    // The symbol of the extended alphabet containing both of them is taken - this is M.
    CHECK_LEVITSKY('M', c1("ACC", 70));
    // The same principle for all other tests - check the table above to find out the consensus base for each second test.
    CHECK_LEVITSKY('G', c1("AGG", 50));
    CHECK_LEVITSKY('R', c1("AGG", 70));
    CHECK_LEVITSKY('T', c1("ATT", 50));
    CHECK_LEVITSKY('W', c1("ATT", 70));
    CHECK_LEVITSKY('U', c1("AUU", 50));
    CHECK_LEVITSKY('W', c1("AUU", 70));

    CHECK_LEVITSKY('A', c1("CAA", 50));
    CHECK_LEVITSKY('M', c1("CAA", 70));
    CHECK_LEVITSKY('G', c1("CGG", 50));
    CHECK_LEVITSKY('S', c1("CGG", 70));
    CHECK_LEVITSKY('U', c1("CUU", 50));
    CHECK_LEVITSKY('Y', c1("CUU", 70));

    CHECK_LEVITSKY('A', c1("GAA", 50));
    CHECK_LEVITSKY('R', c1("GAA", 70));
    CHECK_LEVITSKY('C', c1("GCC", 50));
    CHECK_LEVITSKY('S', c1("GCC", 70));
    CHECK_LEVITSKY('U', c1("GUU", 50));
    CHECK_LEVITSKY('K', c1("GUU", 70));

    CHECK_LEVITSKY('A', c1("TAA", 50));
    CHECK_LEVITSKY('W', c1("TAA", 70));
    CHECK_LEVITSKY('C', c1("TCC", 50));
    CHECK_LEVITSKY('Y', c1("TCC", 70));
    CHECK_LEVITSKY('G', c1("TGG", 50));
    CHECK_LEVITSKY('K', c1("TGG", 70));

    CHECK_LEVITSKY('A', c1("UAA", 50));
    CHECK_LEVITSKY('W', c1("UAA", 70));
    CHECK_LEVITSKY('C', c1("UCC", 50));
    CHECK_LEVITSKY('Y', c1("UCC", 70));
    CHECK_LEVITSKY('G', c1("UGG", 50));
    CHECK_LEVITSKY('K', c1("UGG", 70));

    CHECK_LEVITSKY('N', c1("ACGT", 50));

    // 2.2 The case, when bases from the whole alignment have influence to the consensus base of a certain column
    //
    // A and C in the first column, but A is used, because it has 25% in whole alignment (less than 75% from C)
    CHECK_LEVITSKY('A', cN({"AC", "CC"}, 50));
    // A and C both skipped, because they have 50% in a column (less thatn 51 required).
    // Y is correct, because it has 100% in the column (50% from A and 50% from C)
    CHECK_LEVITSKY('M', cN({"AC", "CC"}, 51));
    CHECK_LEVITSKY('A', cN({"AG", "GG"}, 50));
    CHECK_LEVITSKY('R', cN({"AG", "GG"}, 51));
    CHECK_LEVITSKY('A', cN({"AT", "TT"}, 50));
    CHECK_LEVITSKY('W', cN({"AT", "TT"}, 51));
    CHECK_LEVITSKY('A', cN({"AU", "UU"}, 50));
    CHECK_LEVITSKY('W', cN({"AU", "UU"}, 51));

    CHECK_LEVITSKY('C', cN({"CA", "AA"}, 50));
    CHECK_LEVITSKY('M', cN({"CA", "AA"}, 51));
    CHECK_LEVITSKY('C', cN({"CG", "GG"}, 50));
    CHECK_LEVITSKY('S', cN({"CG", "GG"}, 51));
    CHECK_LEVITSKY('C', cN({"CT", "TT"}, 50));
    CHECK_LEVITSKY('Y', cN({"CT", "TT"}, 51));
    CHECK_LEVITSKY('C', cN({"CU", "UU"}, 50));
    CHECK_LEVITSKY('Y', cN({"CU", "UU"}, 51));

    CHECK_LEVITSKY('G', cN({"GA", "AA"}, 50));
    CHECK_LEVITSKY('R', cN({"GA", "AA"}, 51));
    CHECK_LEVITSKY('G', cN({"GC", "CC"}, 50));
    CHECK_LEVITSKY('S', cN({"GC", "CC"}, 51));
    CHECK_LEVITSKY('G', cN({"GT", "TT"}, 50));
    CHECK_LEVITSKY('K', cN({"GT", "TT"}, 51));
    CHECK_LEVITSKY('G', cN({"GU", "UU"}, 50));
    CHECK_LEVITSKY('K', cN({"GU", "UU"}, 51));

    CHECK_LEVITSKY('T', cN({"TA", "AA"}, 50));
    CHECK_LEVITSKY('W', cN({"TA", "AA"}, 51));
    CHECK_LEVITSKY('T', cN({"TC", "CC"}, 50));
    CHECK_LEVITSKY('Y', cN({"TC", "CC"}, 51));
    CHECK_LEVITSKY('T', cN({"TG", "GG"}, 50));
    CHECK_LEVITSKY('K', cN({"TG", "GG"}, 51));

    CHECK_LEVITSKY('U', cN({"UA", "AA"}, 50));
    CHECK_LEVITSKY('W', cN({"UA", "AA"}, 51));
    CHECK_LEVITSKY('U', cN({"UC", "CC"}, 50));
    CHECK_LEVITSKY('Y', cN({"UC", "CC"}, 51));
    CHECK_LEVITSKY('U', cN({"UG", "GG"}, 50));
    CHECK_LEVITSKY('K', cN({"UG", "GG"}, 51));

    // 3. Three bases mix
    // 3.1 The case, when consensus character is defined only by bases of the column
    CHECK_LEVITSKY('A', cN({"A", "A", "C", "G"}, 50));
    CHECK_LEVITSKY('V', cN({"A", "A", "C", "G"}, 51));
    CHECK_LEVITSKY('A', cN({"A", "A", "C", "T"}, 50));
    CHECK_LEVITSKY('H', cN({"A", "A", "C", "T"}, 51));
    CHECK_LEVITSKY('A', cN({"A", "A", "T", "G"}, 50));
    CHECK_LEVITSKY('D', cN({"A", "A", "T", "G"}, 51));
    CHECK_LEVITSKY('A', cN({"A", "A", "C", "U"}, 50));
    CHECK_LEVITSKY('H', cN({"A", "A", "C", "U"}, 51));
    CHECK_LEVITSKY('A', cN({"A", "A", "U", "G"}, 50));
    CHECK_LEVITSKY('D', cN({"A", "A", "U", "G"}, 51));

    CHECK_LEVITSKY('C', cN({"C", "C", "A", "G"}, 50));
    CHECK_LEVITSKY('V', cN({"C", "C", "A", "G"}, 51));
    CHECK_LEVITSKY('C', cN({"C", "C", "A", "T"}, 50));
    CHECK_LEVITSKY('H', cN({"C", "C", "A", "T"}, 51));
    CHECK_LEVITSKY('C', cN({"C", "C", "T", "G"}, 50));
    CHECK_LEVITSKY('B', cN({"C", "C", "T", "G"}, 51));
    CHECK_LEVITSKY('C', cN({"C", "C", "A", "U"}, 50));
    CHECK_LEVITSKY('H', cN({"C", "C", "A", "U"}, 51));
    CHECK_LEVITSKY('C', cN({"C", "C", "U", "G"}, 50));
    CHECK_LEVITSKY('B', cN({"C", "C", "U", "G"}, 51));

    CHECK_LEVITSKY('G', cN({"G", "G", "A", "C"}, 50));
    CHECK_LEVITSKY('V', cN({"G", "G", "A", "C"}, 51));
    CHECK_LEVITSKY('G', cN({"G", "G", "A", "T"}, 50));
    CHECK_LEVITSKY('D', cN({"G", "G", "A", "T"}, 51));
    CHECK_LEVITSKY('G', cN({"G", "G", "T", "C"}, 50));
    CHECK_LEVITSKY('B', cN({"G", "G", "T", "C"}, 51));
    CHECK_LEVITSKY('G', cN({"G", "G", "A", "U"}, 50));
    CHECK_LEVITSKY('D', cN({"G", "G", "A", "U"}, 51));
    CHECK_LEVITSKY('G', cN({"G", "G", "U", "C"}, 50));
    CHECK_LEVITSKY('B', cN({"G", "G", "U", "C"}, 51));

    CHECK_LEVITSKY('T', cN({"T", "T", "A", "C"}, 50));
    CHECK_LEVITSKY('H', cN({"T", "T", "A", "C"}, 51));
    CHECK_LEVITSKY('T', cN({"T", "T", "A", "G"}, 50));
    CHECK_LEVITSKY('D', cN({"T", "T", "A", "G"}, 51));
    CHECK_LEVITSKY('T', cN({"T", "T", "C", "G"}, 50));
    CHECK_LEVITSKY('B', cN({"T", "T", "C", "G"}, 51));

    CHECK_LEVITSKY('U', cN({"U", "U", "A", "C"}, 50));
    CHECK_LEVITSKY('H', cN({"U", "U", "A", "C"}, 51));
    CHECK_LEVITSKY('U', cN({"U", "U", "A", "G"}, 50));
    CHECK_LEVITSKY('D', cN({"U", "U", "A", "G"}, 51));
    CHECK_LEVITSKY('U', cN({"U", "U", "C", "G"}, 50));
    CHECK_LEVITSKY('B', cN({"U", "U", "C", "G"}, 51));

    // The case when the rare explicit R has higher priority that a single letter T.
    CHECK_LEVITSKY('R', cN({"RT", "TT"}, 50));

    // The case when because of the high threshold W is selected instead of T.
    CHECK_LEVITSKY('T', cN({"AA", "TA"}, 50));  // Low threshold.
    CHECK_LEVITSKY('W', cN({"AA", "TA"}, 90));  // High threshold.

    // 3.2 The case, when bases from the whole alignment have influence to the consensus base of a certain column
    // We have three bases in column, but one of them is to often appeared is second column and not included into mix
    // !! It is becoming impossible to test all possible options, so there is just several of them !!
    CHECK_LEVITSKY('S', cN({"AA", "AA", "CA", "GA"}, 50));
    CHECK_LEVITSKY('V', cN({"AA", "AA", "CA", "GA"}, 51));
    CHECK_LEVITSKY('A', cN({"AC", "AC", "CC", "GC"}, 50));
    CHECK_LEVITSKY('R', cN({"AC", "AC", "CC", "GC"}, 51));
    CHECK_LEVITSKY('A', cN({"AC", "AC", "CC", "TC"}, 50));
    CHECK_LEVITSKY('W', cN({"AC", "AC", "CC", "TC"}, 51));
    CHECK_LEVITSKY('A', cN({"AC", "AC", "CC", "UC"}, 50));
    CHECK_LEVITSKY('W', cN({"AC", "AC", "CC", "UC"}, 51));

    // 4. Four bases mix
    CHECK_LEVITSKY('N', cN({"A", "C", "G", "T"}, 50));
    CHECK_LEVITSKY('N', cN({"A", "C", "G", "T"}, 75));
    CHECK_LEVITSKY('N', cN({"A", "C", "G", "T"}, 100))
    CHECK_LEVITSKY('N', cN({"A", "C", "G", "T"}, 100))
    CHECK_LEVITSKY('W', cN({"AG", "CG", "GC", "TC"}, 50));
    CHECK_LEVITSKY('N', cN({"AA", "CC", "GG", "TT"}, 70));
    CHECK_LEVITSKY('D', cN({"AC", "CC", "GG", "TT"}, 70));  // D has the rarest global percentage (has no C).
}

}  // namespace U2
