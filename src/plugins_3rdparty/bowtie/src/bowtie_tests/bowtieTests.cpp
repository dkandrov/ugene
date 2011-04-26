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

#include "BowtieConstants.h"
#include "bowtieTests.h"

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>


#include <QtCore/QDir>
#include <QtCore/QRegExp>

#include <U2View/DnaAssemblyUtils.h>


/* TRANSLATOR U2::GTest*/

namespace U2 {

#define READS_FILE_NAME_ATTR "reads"
#define INDEX_ATTR "index"
#define PATTERN_FILE_NAME_ATTR "pattern"
#define PATTERN_FILE_FORMAT_ATTR  "pat_format"
#define BUILD_INDEX_ATTR "build"
#define NEGATIVE_ATTR "negative"
#define N_MODE_ATTR "n"
#define V_MODE_ATTR "v"
#define MAQERR_ATTR "maqerr"
#define SEEDLEN_ATTR "seedlen"
#define MAXBTS_ATTR "maxbts"
#define SEED_ATTR "seed"
#define NOMAQROUND_ATTR "nomaqround"
#define NOFW_ATTR "nofw"
#define NORC_ATTR "norc"
#define TRYHARD_ATTR "tryhard"
#define FORMAT_ATTR "format"
#define BEST_ATTR "best"
#define ALL_ATTR "all"

void GTest_Bowtie::init(XMLTestFormat *tf, const QDomElement& el) {
	Q_UNUSED(tf);
	bowtieTask = NULL;
	resultLoadTask = NULL;
	indexName = "";
	readsFileName = "";
	patternFileName = "";
	negativeError = "";
	ma1 = NULL;
	ma2 = NULL;
	usePrebuildIndex = true;
	subTaskFailed = false;
	indexName = el.attribute(INDEX_ATTR);
	format = BaseDocumentFormats::PLAIN_FASTA;
	patternFormat = BaseDocumentFormats::PLAIN_TEXT;
	if(indexName.isEmpty()) {failMissingValue(INDEX_ATTR); return;}
	QString buildStr = el.attribute(BUILD_INDEX_ATTR);
	if(!buildStr.isEmpty()) {
		bool ok = false;
		usePrebuildIndex = ! buildStr.toInt(&ok);
		if(!ok) {failMissingValue(BUILD_INDEX_ATTR); return;}
	}
	readsFileName = el.attribute(READS_FILE_NAME_ATTR);
	if(readsFileName.isEmpty()) {failMissingValue(READS_FILE_NAME_ATTR); return;}
	patternFileName = el.attribute(PATTERN_FILE_NAME_ATTR);
	if(patternFileName.isEmpty()) {failMissingValue(PATTERN_FILE_NAME_ATTR); return;}
	negativeError = el.attribute(NEGATIVE_ATTR);
	QString formatStr = el.attribute(FORMAT_ATTR);
	if(!formatStr.isEmpty()) {
		if(formatStr == "fasta") format = BaseDocumentFormats::PLAIN_FASTA;
		else if(formatStr == "fastq") format = BaseDocumentFormats::FASTQ;
		else { failMissingValue(FORMAT_ATTR); return; }
	}
	QString patternFormatStr = el.attribute(PATTERN_FILE_FORMAT_ATTR);
	if(!patternFormatStr.isEmpty()) {
		if(patternFormatStr == "sam") patternFormat = BaseDocumentFormats::SAM;
		else if(patternFormatStr == "map") patternFormat = BaseDocumentFormats::PLAIN_TEXT;
		else { failMissingValue(PATTERN_FILE_FORMAT_ATTR); return; }
	}

	bool ok = false;
	{
		const QString attr = N_MODE_ATTR;
		
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_N_MISMATCHES, el.attribute(attr).toInt(&ok)); 
			if(!ok) failMissingValue(attr);
		}
	}
	{
		const QString attr = V_MODE_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_V_MISMATCHES, el.attribute(attr).toInt(&ok)); 
			if(!ok) failMissingValue(attr);
		}
	}
	{
		const QString attr = MAQERR_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_MAQERR, el.attribute(attr).toInt(&ok)); 
			if(!ok) failMissingValue(attr);
		}
	}
	{
		const QString attr = SEEDLEN_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_SEED_LEN, el.attribute(attr).toInt(&ok)); 
			if(!ok) failMissingValue(attr);
		}
	}
	{
		const QString attr = SEED_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_SEED, el.attribute(attr).toInt(&ok)); 
			if(!ok) failMissingValue(attr);
		}
	}
	{
		const QString attr = MAXBTS_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_MAXBTS, el.attribute(attr).toInt(&ok)); 
			if(!ok) failMissingValue(attr);
		}
	}
	{
		const QString attr = NOMAQROUND_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_NOMAQROUND, true); 
		}
	}
	{
		const QString attr = NOFW_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_NOFW, true); 
		}
	}
	{
		const QString attr = NORC_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_NORC, true); 
		}
	}
	{
		const QString attr = TRYHARD_ATTR;
		if(!el.attribute(attr).isEmpty()) {
			config.setCustomValue(BowtieTask::OPTION_TRYHARD, true); 
		}
	}
}




void GTest_Bowtie::prepare() {
	if(!usePrebuildIndex) {
		QFileInfo refFile(env->getVar("COMMON_DATA_DIR")+"/"+indexName);
		if(!refFile.exists()) {
			stateInfo.setError(  QString("file not exist %1").arg(refFile.absoluteFilePath()) );
			return;
		}
	}
	QFileInfo readsFile(env->getVar("COMMON_DATA_DIR")+"/"+readsFileName);
	if(!readsFile.exists()) {
		stateInfo.setError(  QString("file not exist %1").arg(readsFile.absoluteFilePath()) );
		return;
	}
	
	readsFileUrl = readsFile.absoluteFilePath();

	QFileInfo patternFile(env->getVar("COMMON_DATA_DIR")+"/"+patternFileName);
	if(!patternFile.exists()) {
		stateInfo.setError(  QString("file not exist %1").arg(patternFile.absoluteFilePath()) );
		return;
	}

    QString tmpDataDir = getTempDataDir();
    if (tmpDataDir.isEmpty()) {
        setError("Can't create tmp data dir!");
        return;
    }
    
    config.shortReadUrls.append(readsFileUrl);
    config.refSeqUrl = GUrl(env->getVar("COMMON_DATA_DIR")+"/"+indexName);
    config.prebuiltIndex = usePrebuildIndex;
    config.resultFileName = GUrl(tmpDataDir+"/"+QString::number(getTaskId()));
    config.algName = BOWTIE_ALG_NAME;
    config.loadResultDocument = true;
    bowtieTask = new BowtieTask(config);

	addSubTask(bowtieTask);
}

QList<Task*> GTest_Bowtie::onSubTaskFinished(Task* subTask) {
	Q_UNUSED(subTask);
	QList<Task*> res;
	if (hasErrors() || subTask->hasErrors() || isCanceled()) {
		subTaskFailed = true;
		return res;
	}
		
	if(subTask == bowtieTask) {
		if(bowtieTask->hasErrors()) {
			subTaskFailed = true;
			return res;
		}
        if (!bowtieTask->isHaveResult()) {
            subTaskFailed = true;
            bowtieTask->setError("Reference assembly failed - no possible alignment found");
            return res;
        }
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(config.resultFileName));
        resultLoadTask = new LoadDocumentTask(BaseDocumentFormats::SAM, config.resultFileName,iof);
        res << resultLoadTask;
		
    }else if (subTask == resultLoadTask) {
        
        Document* doc = resultLoadTask->getDocument();
        if (doc == NULL) {
           setError("Failed to load result document");
           return res;
        }
    
        ma1 =  qobject_cast<MAlignmentObject*> (doc->getObjects().first()->clone());
		
		QFileInfo patternFile(env->getVar("COMMON_DATA_DIR")+"/"+patternFileName);
		IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(patternFile.absoluteFilePath()));
		patternLoadTask = new LoadDocumentTask(patternFormat, patternFile.absoluteFilePath(), iof);
		patternLoadTask->setSubtaskProgressWeight(0);
		res << patternLoadTask;

	} else if(subTask == patternLoadTask) {
		if(patternLoadTask->hasErrors()) {
			subTaskFailed = true;
			return res;
		}
		Document *doc = patternLoadTask->getDocument();
		assert(doc!=NULL);

		if(patternFormat == BaseDocumentFormats::PLAIN_TEXT) {
			QList<GObject*> list = doc->findGObjectByType(GObjectTypes::TEXT);
			if (list.size() == 0) {
				stateInfo.setError(  QString("container of object with type \"%1\" is empty").arg(GObjectTypes::TEXT) );
				return res;
			}
			TextObject* text = qobject_cast<TextObject*>(list.first());
			MAlignment ma;
			parseBowtieOutput(ma, text->getText());
			ma.setName("name");
			ma2 = new MAlignmentObject(ma);
			if(ma2 == NULL) {
				stateInfo.setError(QString("Can't cast GObject to MAlignmentObject"));
				return res;
			}
		} else {
			QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
			if (list.size() == 0) {
				stateInfo.setError(  QString("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT) );
				return res;
			}
			ma2 = qobject_cast<MAlignmentObject*>(list.first());
			if(ma2 == NULL) {
				stateInfo.setError(QString("Can't cast GObject to MAlignmentObject"));
				return res;
			}
		}
	}
	return res;
}

void GTest_Bowtie::run() {
	
	if(subTaskFailed) return;

	const QList<MAlignmentRow> &alignedSeqs1 = ma1->getMAlignment().getRows();
	const QList<MAlignmentRow> &alignedSeqs2 = ma2->getMAlignment().getRows();

	if(alignedSeqs1.count() != alignedSeqs2.count()) {
		stateInfo.setError(QString("Aligned sequences number not matched \"%1\", expected \"%2\"").arg(alignedSeqs1.count()).arg(alignedSeqs2.count()));
		return;
	}

	foreach(const MAlignmentRow &maItem1, alignedSeqs1) {
		bool nameFound = false;
		if(maItem1.getName().compare("reference", Qt::CaseInsensitive)==0) continue;
		foreach(const MAlignmentRow &maItem2, alignedSeqs2) {
			if (maItem1.getName() == maItem2.getName()) {
				nameFound = true;
				int l1 = maItem1.getCoreLength();
				int l2 = maItem2.getCoreLength();
				if (l1 != l2) {
					stateInfo.setError(  QString("Aligned sequences \"%1\" length not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(l1).arg(l2) );
					return;
				}
				if (maItem1.getCore() != maItem2.getCore()) {
					stateInfo.setError(  QString("Aligned sequences \"%1\" not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(QString(maItem1.getCore())).arg(QString(maItem2.getCore())) );
					return;
				}
				
				DNAQuality qual1 = maItem1.getCoreQuality();
				DNAQuality qual2 = maItem1.getCoreQuality();
				if(qual1.type != qual2.type) {
					stateInfo.setError(  QString("Aligned sequences quality type \"%1\" not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(qual1.type).arg(qual2.type) );
				}
				if(qual1.qualCodes != qual2.qualCodes) {
					stateInfo.setError(  QString("Aligned sequences quality \"%1\" not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(QString(qual1.qualCodes)).arg(QString(qual2.qualCodes)) );
					return;
				}
			}
		}
		if (!nameFound) {
			stateInfo.setError(  QString("aligned sequence not found \"%1\"").arg(maItem1.getName()) );
		}
	}
}

Task::ReportResult GTest_Bowtie::report() {
	if(!negativeError.isEmpty()) {
		if(hasSubtasksWithErrors()) {
			return ReportResult_Finished;
		} else {
			setError(QString("Negative test failed: error string is empty, expected error \"%1\"").arg(negativeError));
		}
	} else {
		propagateSubtaskError();
	}
	return ReportResult_Finished;
}

void GTest_Bowtie::cleanup()
{
	
    // delete index
    if(!usePrebuildIndex) {
		QString prefix = env->getVar("TEMP_DATA_DIR")+"/"+QString::number(getTaskId());
		QStringList files(QStringList() << prefix+".1.ebwt" << prefix+".2.ebwt" 
			<< prefix+".3.ebwt" << prefix+".4.ebwt" << prefix+".rev.1.ebwt" << prefix+".rev.2.ebwt");
		foreach(QString file, files) {
			QFileInfo tmpFile(file);
			if(tmpFile.exists()) {
				ioLog.trace(QString("Deleting index file \"%1\"").arg(tmpFile.absoluteFilePath()));
				QFile::remove(tmpFile.absoluteFilePath());
			} 
		}
	}
    //delete tmp result
    QFileInfo tmpResult(config.resultFileName.getURLString());
    if (tmpResult.exists()) {
        ioLog.trace(QString("Deleting tmp result file :%1").arg(tmpResult.absoluteFilePath()));
        QFile::remove(tmpResult.absoluteFilePath());
    }

	delete ma1;
	if(patternFormat == BaseDocumentFormats::PLAIN_TEXT)
		delete ma2;
	ma1 = NULL;
	ma2 = NULL;
}

QList<DNASequence> GTest_Bowtie::dnaObjList_to_dnaList(QList<GObject*> dnaSeqs) {

	QList<DNASequence> res;
	int seqCount = dnaSeqs.count();
	DNASequenceObject *seq = qobject_cast<DNASequenceObject *>(dnaSeqs[0]);
	MAlignment ma("Alignment",seq->getAlphabet());
	for(int i=0; i<seqCount; i++) {
		seq = qobject_cast<DNASequenceObject *>(dnaSeqs[i]);
		if(seq == NULL) {
			stateInfo.setError(  QString("Can't cast GObject to DNASequenceObject") );
			return res;
		}
		res.append(seq->getDNASequence());
	}
	return res;
}

void GTest_Bowtie::parseBowtieOutput( MAlignment& result, QString text )
{
	QRegExp rx("(\\S+)\\s+([\\+\\-])\\s+\\S+\\s+(\\d+)\\s+(\\S+)\\s(\\S+)(?!\\n)");
	int pos = 0;
	while ((pos = rx.indexIn(text, pos)) != -1) {
		QString name = rx.cap(1);
		bool isReverseComplement = rx.cap(2)[0] == '-';
		int offset = rx.cap(3).toInt();
		QByteArray sequence = rx.cap(4).toAscii();
		QByteArray quality = rx.cap(5).toAscii();
		if(isReverseComplement) {
            DNAAlphabet *al = AppContext::getDNAAlphabetRegistry()->findAlphabet(sequence);
            if(al == NULL) {
                stateInfo.setError(QString("Can't find alphabet for sequence \"%1\"").arg(QString(sequence)));
                return;
            }
            DNATranslation* tr = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(al);
            if(tr == NULL) {
                stateInfo.setError(QString("Can't translation for alphabet \"%1\"").arg(al->getName()));
                return;
            }
            TextUtils::translate(tr->getOne2OneMapper(), sequence.data(), sequence.size());
            TextUtils::reverse(sequence.data(), sequence.size());
            TextUtils::reverse(quality.data(), quality.size());
		}
		MAlignmentRow row(name, sequence, offset);
		row.setQuality(DNAQuality(quality));
		result.addRow(row);
		
		pos += rx.matchedLength();

	}
}

GTest_Bowtie::~GTest_Bowtie()
{
	cleanup();
}

QString GTest_Bowtie::getTempDataDir()
{
    QString dir(env->getVar("TEMP_DATA_DIR"));
    if (!QDir(dir).exists()) {
        bool ok = QDir::root().mkpath(dir);
        if (!ok) {
               return QString();
        }
    }
    return dir;
}

QList<XMLTestFactory*> BowtieTests::createTestFactories() {
	QList<XMLTestFactory*> res;
	res.append(GTest_Bowtie::createFactory());
	return res;
}
}

