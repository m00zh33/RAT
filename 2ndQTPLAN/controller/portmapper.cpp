#include "portmapper.h"

portmapper::portmapper(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::portmapper();
    ui->setupUi(this);
    controller *parentCtrl =
            dynamic_cast<controller*>(parent);
//    ^^ this will prolly not work cause parent is not set yet
    parentCtrl->portPopulate(ui->lstBinding);
    this->show();
}

portmapper::~portmapper()
{
    delete ui;
}

void portmapper::on_cmdApply_clicked()
{
    controller *parent = dynamic_cast<controller*>(this->parent());
    parent->portClear();

    QPointer<QAbstractItemModel> model = ui->lstBinding->model();
    QModelIndex index;
    controller *parentCtrl = dynamic_cast<controller*>(this->parent());
    QHostAddress bindIp, destIp;
    quint16 bindPort, destPort;

    for(quint16 i = 0; i < ui->lstBinding->rowCount(); ++i)
    {
        index = model->index(i, 0, QModelIndex());
        bindIp.setAddress(index.data(Qt::DisplayRole).toString());
        if (bindIp.isNull()) continue;
        index = model->index(i, 2, QModelIndex());
        destIp.setAddress(index.data(Qt::DisplayRole).toString());
        if (destIp.isNull()) continue;
        index = model->index(i, 1, QModelIndex());
        bindPort = index.data(Qt::DisplayRole).toInt();
        index = model->index(i, 3, QModelIndex());
        destPort = index.data(Qt::DisplayRole).toInt();
        parentCtrl->portMapAdd(bindIp, bindPort, destIp, destPort);
    }

    this->hide();
    this->deleteLater();
}

void portmapper::on_cmdInsertRow_clicked()
{
    ui->lstBinding->insertRow(ui->lstBinding->rowCount());
}

void portmapper::on_cmdDelRow_clicked()
{
    int currentRow = ui->lstBinding->currentRow();
    QString bindIp;
    quint16 bindPort;
    QPointer<QAbstractItemModel> model = ui->lstBinding->model();
    QModelIndex index;
    index = model->index(currentRow, 0, QModelIndex());
    bindIp = index.data(Qt::DisplayRole).toString();
    index = model->index(currentRow, 1, QModelIndex());
    bindPort = index.data(Qt::DisplayRole).toInt();
    controller *parentCtrl = dynamic_cast<controller*>(this->parent());
    parentCtrl->portMapDel(bindIp, bindPort);
    ui->lstBinding->removeRow(currentRow);
}
