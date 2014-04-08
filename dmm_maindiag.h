#ifndef DMM_MainDiag_H
#define DMM_MainDiag_H

#include "ui_dmm_maindiag.h"
#include <QSignalMapper>

class DMM_MainDiag : public QDialog
{
  Q_OBJECT
  public:
      DMM_MainDiag ( QWidget *parent=0);
      ~DMM_MainDiag();

//public slots:
//public signals:

private slots:
  void slotConnectClicked();
  void timer_task();
  void updateTimer();
  void getID();
  void updateMSG();
  void updateDBG();
  void getMeasurement();
  void setMeasurementType(QString type);
//private signals:    
    
    
private:
    Ui::DMM_MainDiag ui;
    void Instr_init();
    QString msg;
    bool waitformeas;

    QSignalMapper *signalMapper;
    
 };

#endif
