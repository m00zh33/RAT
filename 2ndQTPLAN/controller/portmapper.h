class portmapper;

#ifndef PORTMAPPER_H
#define PORTMAPPER_H

#include <QtGui>
#include "ui_portmapper.h"
#include "controller.h"

namespace Ui
{
    class portmapper;
}

class portmapper : public QDialog
{
    Q_OBJECT
public:

    portmapper(QWidget *parent);
    ~portmapper();
private:
    Ui::portmapper *ui;

private slots:
    void on_cmdDelRow_clicked();
    void on_cmdInsertRow_clicked();
    void on_cmdApply_clicked();
};

#endif // PORTMAPPER_H
