#ifndef DMM_MainDiag_H
#define DMM_MainDiag_H

#include "ui_dmm_maindiag.h"

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
//private signals:    
    
    
private:
    Ui::DMM_MainDiag ui;
    void Instr_init();
    void updateTE();
    
 };

#endif