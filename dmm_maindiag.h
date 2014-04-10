#ifndef DMM_MainDiag_H
#define DMM_MainDiag_H

#include "ui_dmm_maindiag.h"
#include <QSignalMapper>
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class DMM_MainDiag : public QDialog
{
  Q_OBJECT
public:
  DMM_MainDiag ( QWidget *parent=0);
  ~DMM_MainDiag();

  //public slots:
  //public signals:
private:
  Ui::DMM_MainDiag ui;
  void Instr_init();
  void Disp_val();
  QString msg;

  enum eDMMstate {idle,waitformeas,cont_meas};
  eDMMstate DMMstate;
  QSignalMapper *signalMapperMT,*signalMapperRE,*signalMapperDCAC,*signalMapperMeas;

  void init_curve();
  QwtPlotCurve *curve1;
  double y[101];
  double x[101];
  int xpos;


private slots:
  void slotConnectClicked();
  void timer_task();
  void updateTimer();
  void getID();
  void getSettings();
  void updateMSG();
  void updateDBG();
  void getMeasurement(int state);
  void setMeasurementType(QString type);
  void setResolution(int res);
  void setDCAC(QString DCAC);
  void Disp_off();
  //private signals:

};

#endif
