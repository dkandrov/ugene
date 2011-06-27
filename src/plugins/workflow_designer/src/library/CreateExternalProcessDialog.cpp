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

#include "CreateExternalProcessDialog.h"

#include <QtGui/QMessageBox>

#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/ExternalToolCfg.h>
#include <U2Designer/DelegateEditors.h>
#include "WorkflowEditorDelegates.h"
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>


namespace U2 {

class CreateExternalProcessDialog;

class CfgExternalToolItem {
public:
    CfgExternalToolItem()  {
        dfr = AppContext::getDocumentFormatRegistry();
        dtr = Workflow::WorkflowEnv::getDataTypeRegistry();

        delegateForTypes = NULL;
        delegateForFormats = NULL;
        itemData.type = BaseTypes::DNA_SEQUENCE_TYPE()->getId();
        itemData.format = BaseDocumentFormats::PLAIN_FASTA;
    }
    ~CfgExternalToolItem() {
        delete delegateForTypes;
        delete delegateForFormats;
    }

    QString getDataType() const {return itemData.type;}
    void setDataType(const QString& id) {
        itemData.type = id;
    }

    QString getName() const {return itemData.attrName;}
    void setName(const QString &_name) {itemData.attrName = _name;}

    QString getFormat() const {return itemData.format;}
    void setFormat(const QString & f) {itemData.format = f;}

    QString getDescription() const {return itemData.description;}
    void setDescription(const QString & _descr) {itemData.description = _descr;}

    PropertyDelegate *delegateForTypes;
    PropertyDelegate *delegateForFormats;

    DataConfig itemData;

private:
    DocumentFormatRegistry *dfr;
    DataTypeRegistry *dtr;
};


class CfgExternalToolModel: public QAbstractTableModel {
public:
    CfgExternalToolModel(QObject *obj = NULL): QAbstractTableModel(obj) {
        init();
        /*CfgListItem *newItem = new CfgListItem();
        newItem->delegateForTypes = new ComboBoxDelegate(types);
        items.append(newItem);*/
    }

    int rowCount(const QModelIndex & /* = QModelIndex */) const{
        return items.size();
    }

    int columnCount(const QModelIndex & /* = QModelIndex */) const {
        return 4;
    }

    Qt::ItemFlags flags(const QModelIndex &) const{
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }   

    CfgExternalToolItem* getItem(const QModelIndex &index) const {
        return items.at(index.row());
    }

    QList<CfgExternalToolItem*> getItems() const {
        return items;
    }

    QVariant data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const {
        CfgExternalToolItem *item = getItem(index);
        int col = index.column();

        switch(role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                if(col == 0) return item->getName();
                else if(col == 1) return item->delegateForTypes->getDisplayValue(item->getDataType());
                else if(col == 2) return item->delegateForFormats->getDisplayValue(item->getFormat());
                else if(col == 3) return item->getDescription();
                else return QVariant();
            case DelegateRole:
                if(col == 1) return qVariantFromValue<PropertyDelegate*>(item->delegateForTypes);
                else if(col == 2) return qVariantFromValue<PropertyDelegate*>(item->delegateForFormats);
                else return QVariant();
            case Qt::EditRole:
            case ConfigurationEditor::ItemValueRole:
                if(col == 1) return item->getDataType();
                else if(col == 2) return item->getFormat();
                else return QVariant();
            default:
                return QVariant();
        }
    }

    void createFormatDelegate(const QString &newType, CfgExternalToolItem *item) {
        PropertyDelegate *delegate;
        QString format;
        if(newType == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
            delegate =  new ComboBoxDelegate(seqFormatsW);
            format = seqFormatsW.values().first().toString();
        } else if(newType == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
            delegate =  new ComboBoxDelegate(msaFormatsW);
            format = msaFormatsW.values().first().toString();
        } else if(newType == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
            delegate =  new ComboBoxDelegate(annFormatsW);
            format = annFormatsW.values().first().toString();
        } else if(newType == SEQ_WITH_ANNS){
            delegate =  new ComboBoxDelegate(annFormatsW);
            format = annFormatsW.values().first().toString();
        } else {
            return;
        }
        item->setFormat(format);
        item->delegateForFormats = delegate;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) {
        int col = index.column();
        CfgExternalToolItem * item = getItem(index);
        switch (role) {
        case Qt::EditRole:
        case ConfigurationEditor::ItemValueRole:
            if(col == 0) {
                if(item->getName() != value.toString()) {
                    item->setName(value.toString());
                } 
            }else if(col == 1) {
                QString newType = value.toString();
                if(item->getDataType() != newType) {
                    if(!newType.isEmpty()) {
                        item->setDataType(newType);
                        createFormatDelegate(newType, item);
                    }
                }
            } else if(col == 2) {
                if(item->getFormat() != value.toString() && !value.toString().isEmpty())  {
                    item->setFormat(value.toString());
                }
            } else if(col == 3) {
                if(item->getDescription() != value.toString()) {
                    item->setDescription(value.toString());
                }
            }
            emit dataChanged(index, index);
        }
        return true;
    }

    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch(section) {
                case 0: return CreateExternalProcessDialog::tr("Name for command line parameter");
                case 1: return CreateExternalProcessDialog::tr("Type");
                case 2: return CreateExternalProcessDialog::tr("Format");
                case 3: return CreateExternalProcessDialog::tr("Description");
                default: return QVariant();
            }
        }
        return QVariant();
    }

    bool insertRows ( int row, int count = 0, const QModelIndex & parent = QModelIndex() )  {
        Q_UNUSED(row);
        Q_UNUSED(count);
        beginInsertRows(parent, items.size(), items.size());
        CfgExternalToolItem *newItem = new CfgExternalToolItem();
        newItem->delegateForTypes = new ComboBoxDelegate(types);
        newItem->delegateForFormats = new ComboBoxDelegate(seqFormatsW);
        items.append(newItem);
        endInsertRows();
        return true;
    }

    bool removeRows(int row, int count = 0, const QModelIndex & parent = QModelIndex()) {
        Q_UNUSED(count);
        if(row >= 0 && row < items.size()) {
            beginRemoveRows(parent, row, row);
            items.removeAt(row);
            endRemoveRows();
            return true;
        } else {
            return false;
        }
    }

    void init() {
        initTypes();
        initFormats();
    }
    void initFormats() {
        QList<DocumentFormatId> ids = AppContext::getDocumentFormatRegistry()->getRegisteredFormats();

        DocumentFormatConstraints seqWrite;
        seqWrite.supportedObjectTypes+=GObjectTypes::SEQUENCE;
        seqWrite.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        seqWrite.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);

        DocumentFormatConstraints seqRead;
        seqRead.supportedObjectTypes+=GObjectTypes::SEQUENCE;
        seqRead.addFlagToSupport(DocumentFormatFlag_SupportStreaming);
        seqRead.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);

        DocumentFormatConstraints msaWrite;
        msaWrite.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
        msaWrite.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        msaWrite.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);

        DocumentFormatConstraints msaRead;
        msaRead.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
        msaRead.addFlagToSupport(DocumentFormatFlag_SupportStreaming);
        msaRead.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);

        DocumentFormatConstraints annWrite;
        annWrite.supportedObjectTypes+=GObjectTypes::ANNOTATION_TABLE;
        annWrite.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        annWrite.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);

        DocumentFormatConstraints annRead;
        annRead.supportedObjectTypes+=GObjectTypes::ANNOTATION_TABLE;
        annRead.addFlagToSupport(DocumentFormatFlag_SupportStreaming);
        annRead.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);

        foreach(const DocumentFormatId& id, ids) {
            DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(id);

            if (df->checkConstraints(seqWrite)) {
                seqFormatsW[df->getFormatName()] = df->getFormatId();
            }

            if (df->checkConstraints(seqRead)) {
                seqFormatsR[df->getFormatName()] = df->getFormatId();
            }

            if (df->checkConstraints(msaWrite)) {
                msaFormatsW[df->getFormatName()] = df->getFormatId();
            }

            if (df->checkConstraints(msaRead)) {
                msaFormatsR[df->getFormatName()] = df->getFormatId();
            } 

            if (df->checkConstraints(annWrite)) {
                annFormatsW[df->getFormatName()] = df->getFormatId();
            }

            if (df->checkConstraints(annRead)) {
                annFormatsR[df->getFormatName()] = df->getFormatId();
            }
        }
    }

    void initTypes() {
        DataTypePtr ptr = BaseTypes::DNA_SEQUENCE_TYPE();
        types[ptr->getDisplayName()] = ptr->getId();

        ptr = BaseTypes::ANNOTATION_TABLE_TYPE();
        types[ptr->getDisplayName()] = ptr->getId();

        ptr = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        types[ptr->getDisplayName()] = ptr->getId();

        types["Sequence with annotations"] = SEQ_WITH_ANNS;
    }

private:
    QList<CfgExternalToolItem*> items;
    QVariantMap types;
    QVariantMap seqFormatsW;
    QVariantMap msaFormatsW;
    QVariantMap annFormatsW;
    QVariantMap seqFormatsR;
    QVariantMap msaFormatsR;
    QVariantMap annFormatsR;
};


class AttributeItem {
public:
    QString getName() const {return name;}
    void setName(const QString& _name) {name = _name;}
    QString getDataType() const {return type;}
    void setDataType(const QString &_type) {type = _type;}
    QString getDescription() const {return description;}
    void setDescription(const QString &_description) {description = _description;}
private:
    QString name;
    QString type;
    QString description;

};

class CfgExternalToolModelAttributes: public QAbstractTableModel {
public:
    CfgExternalToolModelAttributes() {
        types["URL"] = "URL";
        types["String"] = "String";
        types["Number"] = "Number";
        types["Boolean"] = "Boolean";
        delegate = new ComboBoxDelegate(types);
    }
    ~CfgExternalToolModelAttributes() {
        foreach(AttributeItem* item, items) {
            delete item;
        }
    }
    int rowCount(const QModelIndex & /* = QModelIndex */) const{
        return items.size();
    }

    int columnCount(const QModelIndex & /* = QModelIndex */) const {
        return 3;
    }

    Qt::ItemFlags flags(const QModelIndex &) const{
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }   

    AttributeItem* getItem(const QModelIndex &index) const {
        return items.at(index.row());
    }

    QList<AttributeItem*> getItems() const {
        return items;
    }

    QVariant data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const {
        AttributeItem *item = getItem(index);
        int col = index.column();

        switch(role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                if(col == 0) return item->getName();
                else if(col == 1) return delegate->getDisplayValue(item->getDataType());
                else if(col == 2) return item->getDescription();
                else return QVariant();
            case DelegateRole:
                if(col == 1) return qVariantFromValue<PropertyDelegate*>(delegate);
                else return QVariant();
            case Qt::EditRole:
            case ConfigurationEditor::ItemValueRole:
                if(col == 1) return item->getDataType();
                else return QVariant();
            default:
                return QVariant();
        }
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) {
        int col = index.column();
        AttributeItem * item = getItem(index);
        switch (role) {
        case Qt::EditRole:
        case ConfigurationEditor::ItemValueRole:
            if(col == 0) {
                if(item->getName() != value.toString()) {
                    item->setName(value.toString());
                } 
            }else if(col == 1) {
                QString newType = value.toString();
                if(item->getDataType() != newType) {
                    if(!newType.isEmpty()) {
                        item->setDataType(newType);
                    }
                }
            } else if(col == 2) {
                if(item->getDescription() != value.toString()) {
                    item->setDescription(value.toString());
                }
            }
            emit dataChanged(index, index);
        }
        return true;
    }

    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch(section) {
                case 0: return CreateExternalProcessDialog::tr("Name");
                case 1: return CreateExternalProcessDialog::tr("Type");
                case 2: return CreateExternalProcessDialog::tr("Description");
                default: return QVariant();
            }
        }
        return QVariant();
    }

    bool insertRows ( int row, int count = 0, const QModelIndex & parent = QModelIndex() )  {
        Q_UNUSED(row);
        Q_UNUSED(count);
        beginInsertRows(parent, items.size(), items.size());
        AttributeItem *newItem = new AttributeItem();
        newItem->setDataType("String");
        items.append(newItem);
        endInsertRows();
        return true;
    }

    bool removeRows(int row, int count = 0, const QModelIndex & parent = QModelIndex()) {
        Q_UNUSED(count);
        if(row >= 0 && row < items.size()) {
            beginRemoveRows(parent, row, row);
            items.removeAt(row);
            endRemoveRows();
            return true;
        } else {
            return false;
        }
    }

private:
    QList<AttributeItem*> items;
    PropertyDelegate *delegate;
    QVariantMap types;
};



CreateExternalProcessDialog::CreateExternalProcessDialog(QWidget *p, ExternalProcessConfig *cfg):QWizard(p) {
    ui.setupUi(this);
    connect(ui.addInputButton, SIGNAL(clicked()), SLOT(sl_addInput()));
    connect(ui.addOutputButton, SIGNAL(clicked()), SLOT(sl_addOutput()));
    connect(ui.deleteInputButton, SIGNAL(clicked()), SLOT(sl_deleteInput()));
    connect(ui.deleteOutputButton, SIGNAL(clicked()), SLOT(sl_deleteOutput()));
    connect(ui.addAttributeButton, SIGNAL(clicked()), SLOT(sl_addAttribute()));
    connect(ui.deleteAttributeButton, SIGNAL(clicked()), SLOT(sl_deleteAttribute()));
    //connect(button(QWizard::FinishButton), SIGNAL(clicked()), SLOT(sl_OK()));
    //connect(button(QWizard::NextButton), SIGNAL(clicked()), SLOT(sl_generateTemplateString()));

    QFontMetrics info(ui.descr1TextEdit->font());
    ui.descr1TextEdit->setFixedHeight(info.height() * 4);
    ui.descr2TextEdit->setFixedHeight(info.height() * 4);
    ui.descr3TextEdit->setFixedHeight(info.height() * 4);
    ui.descr4TextEdit->setFixedHeight(info.height() * 4);

    ui.inputTableView->setModel(new CfgExternalToolModel());
    ui.outputTableView->setModel(new CfgExternalToolModel());
    ui.attributesTableView->setModel(new CfgExternalToolModelAttributes());

    ui.inputTableView->setItemDelegate(new ProxyDelegate());
    ui.outputTableView->setItemDelegate(new ProxyDelegate());
    ui.attributesTableView->setItemDelegate(new ProxyDelegate());

    ui.inputTableView->horizontalHeader()->setStretchLastSection(true);
    ui.outputTableView->horizontalHeader()->setStretchLastSection(true);
    ui.attributesTableView->horizontalHeader()->setStretchLastSection(true);
    ui.inputTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui.outputTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui.attributesTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    int ind = 0;
    foreach(const DataConfig &dataCfg, cfg->inputs) {
        ui.inputTableView->model()->insertRow(0, QModelIndex());
        QModelIndex index = ui.inputTableView->model()->index(ind,0);
        ui.inputTableView->model()->setData(index, dataCfg.attrName);
        index = ui.inputTableView->model()->index(ind,1);
        ui.inputTableView->model()->setData(index, dataCfg.type);
        index = ui.inputTableView->model()->index(ind,2);
        ui.inputTableView->model()->setData(index, dataCfg.format);
        index = ui.inputTableView->model()->index(ind,3);
        ui.inputTableView->model()->setData(index, dataCfg.description);
        ind++;
    }

    ind = 0;
    foreach(const DataConfig &dataCfg, cfg->outputs) {
        ui.outputTableView->model()->insertRow(0, QModelIndex());
        QModelIndex index = ui.outputTableView->model()->index(ind,0);
        ui.outputTableView->model()->setData(index, dataCfg.attrName);
        index = ui.outputTableView->model()->index(ind,1);
        ui.outputTableView->model()->setData(index, dataCfg.type);
        index = ui.outputTableView->model()->index(ind,2);
        ui.outputTableView->model()->setData(index, dataCfg.format);
        index = ui.outputTableView->model()->index(ind,3);
        ui.outputTableView->model()->setData(index, dataCfg.description);
        ind++;
    }

    ind = 0;
    foreach(const AttributeConfig &attrCfg, cfg->attrs) {
        ui.attributesTableView->model()->insertRow(0, QModelIndex());
        QModelIndex index = ui.attributesTableView->model()->index(ind,0);
        ui.attributesTableView->model()->setData(index, attrCfg.attrName);
        index = ui.attributesTableView->model()->index(ind,1);
        ui.attributesTableView->model()->setData(index, attrCfg.type);
        index = ui.attributesTableView->model()->index(ind,2);
        ui.attributesTableView->model()->setData(index, attrCfg.description);
        ind++;
    }

    ui.nameLineEdit->setText(cfg->name);
    ui.descriptionTextEdit->setText(cfg->description);
    ui.templateLineEdit->setText(cfg->cmdLine);
    ui.prompterTextEdit->setText(cfg->templateDescription);

    editing = true;
}

void CreateExternalProcessDialog::sl_addInput() {
    ui.inputTableView->model()->insertRow(0, QModelIndex());
}

void CreateExternalProcessDialog::sl_addOutput() {
    ui.outputTableView->model()->insertRow(0, QModelIndex());
}

void CreateExternalProcessDialog::sl_deleteInput() {
    QModelIndex index = ui.inputTableView->currentIndex();
    ui.inputTableView->model()->removeRow(index.row());
}

void CreateExternalProcessDialog::sl_deleteOutput() {
    QModelIndex index = ui.outputTableView->currentIndex();
    ui.outputTableView->model()->removeRow(index.row());
}

void CreateExternalProcessDialog::sl_addAttribute() {
    ui.attributesTableView->model()->insertRow(0, QModelIndex());
}

void CreateExternalProcessDialog::sl_deleteAttribute() {
    QModelIndex index = ui.attributesTableView->currentIndex();
    ui.attributesTableView->model()->removeRow(index.row());
}

CreateExternalProcessDialog::CreateExternalProcessDialog( QWidget *p /*= NULL*/ ): QWizard(p) {
    ui.setupUi(this);
    connect(ui.addInputButton, SIGNAL(clicked()), SLOT(sl_addInput()));
    connect(ui.addOutputButton, SIGNAL(clicked()), SLOT(sl_addOutput()));
    connect(ui.deleteInputButton, SIGNAL(clicked()), SLOT(sl_deleteInput()));
    connect(ui.deleteOutputButton, SIGNAL(clicked()), SLOT(sl_deleteOutput()));
    connect(ui.addAttributeButton, SIGNAL(clicked()), SLOT(sl_addAttribute()));
    connect(ui.deleteAttributeButton, SIGNAL(clicked()), SLOT(sl_deleteAttribute()));
    //connect(button(QWizard::FinishButton), SIGNAL(clicked()), SLOT(sl_OK()));
    connect(button(QWizard::NextButton), SIGNAL(clicked()), SLOT(sl_generateTemplateString()));

    ui.inputTableView->setModel(new CfgExternalToolModel());
    ui.outputTableView->setModel(new CfgExternalToolModel());
    ui.attributesTableView->setModel(new CfgExternalToolModelAttributes());

    ui.inputTableView->setItemDelegate(new ProxyDelegate());
    ui.outputTableView->setItemDelegate(new ProxyDelegate());
    ui.attributesTableView->setItemDelegate(new ProxyDelegate());

    ui.inputTableView->horizontalHeader()->setStretchLastSection(true);
    ui.outputTableView->horizontalHeader()->setStretchLastSection(true);
    ui.attributesTableView->horizontalHeader()->setStretchLastSection(true);

    ui.inputTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui.outputTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui.attributesTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    QFontMetrics info(ui.descr1TextEdit->font());
    ui.descr1TextEdit->setFixedHeight(info.height() * 4);
    ui.descr2TextEdit->setFixedHeight(info.height() * 4);
    ui.descr3TextEdit->setFixedHeight(info.height() * 4);
    ui.descr4TextEdit->setFixedHeight(info.height() * 4);
    editing = false;
}

void CreateExternalProcessDialog::accept() {
    CfgExternalToolModel *model;
    cfg = new ExternalProcessConfig();
    cfg->name = ui.nameLineEdit->text();
    cfg->description = ui.descriptionTextEdit->toPlainText();
    cfg->templateDescription = ui.prompterTextEdit->toPlainText();

    model = static_cast<CfgExternalToolModel*>(ui.inputTableView->model());
    foreach(CfgExternalToolItem *item, model->getItems()) {
        cfg->inputs << item->itemData;
    }

    model = static_cast<CfgExternalToolModel*>(ui.outputTableView->model());
    foreach(CfgExternalToolItem *item, model->getItems()) {
        cfg->outputs << item->itemData;
    }

    CfgExternalToolModelAttributes *aModel = static_cast<CfgExternalToolModelAttributes*>(ui.attributesTableView->model());
    foreach(AttributeItem *item, aModel->getItems()) {
        AttributeConfig attributeData;
        attributeData.attrName = item->getName();
        attributeData.type = item->getDataType();
        attributeData.description = item->getDescription();
        cfg->attrs << attributeData;
    }

    cfg->cmdLine = ui.templateLineEdit->text();

    if(!validate()) {
        return;
    }
    
    QString str = HRSchemaSerializer::actor2String(cfg);
    QString dir = WorkflowSettings::getExternalToolDirectory();
    QDir d(dir);
    if(!d.exists()) {
        d.mkdir(dir);
    }
    QFile file(dir + cfg->name + ".etc");
    file.open(QIODevice::WriteOnly);
    file.write(str.toAscii());
    file.close();

    done(QDialog::Accepted);
}

bool CreateExternalProcessDialog::validate() {
    QString title = tr("Create wrapper for external tool");
    if(cfg->inputs.isEmpty() && cfg->outputs.isEmpty())  {
        QMessageBox::critical(this, title, tr("Nor input data nor output data was set"));
        return false;
    }

    if(cfg->cmdLine.isEmpty()) {
        QMessageBox::critical(this, title, tr("Set command line to run external tool"));
        return false;
    }

    if(cfg->name.isEmpty()) {
        QMessageBox::critical(this, title, tr("Set name for new element"));
        return false;
    }

    QRegExp invalidSymbols("[\\.,:;\\?]");
    if(cfg->name.contains(invalidSymbols)) {
        QMessageBox::critical(this, title, tr("Invalid symbols in element name"));
        return false;
    }

    if(WorkflowEnv::getProtoRegistry()->getProto(cfg->name) && !editing) {
        QMessageBox::critical(this, title, tr("Element with this name already exists"));
        return false;
    }

    invalidSymbols = QRegExp("\\W");
    QStringList nameList;
    foreach(const DataConfig & dc, cfg->inputs) {
        if(dc.attrName.isEmpty()) {
            QMessageBox::critical(this, title, tr("For one or more parameters name was not set"));
            return false;
        } 
        if(dc.attrName.contains(invalidSymbols)) {
            QMessageBox::critical(this, title, tr("Invalid symbols in name").arg(dc.attrName));
            return false;
        }
        nameList << dc.attrName;
    }
    foreach(const DataConfig & dc, cfg->outputs) {
        if(dc.attrName.isEmpty()) {
            QMessageBox::critical(this, title, tr("For one or more parameters name was not set"));
            return false;
        } 
        if(dc.attrName.contains(invalidSymbols)) {
            QMessageBox::critical(this, title, tr("Invalid symbols in name").arg(dc.attrName));
            return false;
        }
        nameList << dc.attrName;
    }
    foreach(const AttributeConfig & ac, cfg->attrs) {
        if(ac.attrName.isEmpty()) {
            QMessageBox::critical(this, title, tr("For one or more parameters name was not set"));
            return false;
        } 
        if(ac.attrName.contains(invalidSymbols)) {
            QMessageBox::critical(this, title, tr("Invalid symbols in name").arg(ac.attrName));
            return false;
        }
        nameList << ac.attrName;
    }

    if(nameList.removeDuplicates() > 0) {
        QMessageBox::critical(this, title, tr("The same name of element pentameters was found"));
        return false;
    }

    foreach(const QString &str, nameList) {
        if(!cfg->cmdLine.contains("$" + str)) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(title);
            msgBox.setText(tr("You don't use parameter %1 in template string. Continue?").arg(str));
            msgBox.addButton(tr("Continue"), QMessageBox::ActionRole);
            QPushButton *cancel = msgBox.addButton(tr("Abort"), QMessageBox::ActionRole);
            msgBox.exec();
            if(msgBox.clickedButton() == cancel) {
                return false;
            }
        }
    }

    return true;
}

void CreateExternalProcessDialog::sl_generateTemplateString() {
    QString cmd = "<My tool>";

    CfgExternalToolModel *model = static_cast<CfgExternalToolModel*>(ui.inputTableView->model());
    foreach(CfgExternalToolItem *item, model->getItems()) {
        cmd += " $" + item->itemData.attrName;
    }

    model = static_cast<CfgExternalToolModel*>(ui.outputTableView->model());
    foreach(CfgExternalToolItem *item, model->getItems()) {
        cmd += " $" + item->itemData.attrName;
    }

    CfgExternalToolModelAttributes *aModel = static_cast<CfgExternalToolModelAttributes*>(ui.attributesTableView->model());
    int i = 0;
    foreach(AttributeItem *item, aModel->getItems()) {
        i++;
        cmd +=  " -p" + QString::number(i) + " $" + item->getName();
    }

    ui.templateLineEdit->setText(cmd);
}

}
