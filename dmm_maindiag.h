#ifndef DMM_MainDiag_H
#define DMM_MainDiag_H

#include "ui_dmm_maindiag.h"
// Removed QSignalMapper include - replaced with modern lambda connections
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include "m2550_access.h" // Include the header for M2550_access class

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
  void show_value(const QString &raw, double val);
  QString base_unit() const;
  QString curType; // measurement type as selected in the GUI: VOLT, CURR, RESI, CAP, FREQ
  static const int MAX_LCD_DIGITS = 10; // above this readings are shown in scientific notation
  QString msg;

  enum eDMMstate {idle,waitformeas,cont_meas};
  eDMMstate DMMstate;
  // Removed QSignalMapper declarations - replaced with modern lambda connections

  void init_curve();
  void update_curve();
  std::unique_ptr<QChart> chart;      // Using smart pointer for QChart
  std::unique_ptr<QLineSeries> curve1; // Using smart pointer for QLineSeries  
  std::unique_ptr<QValueAxis> axisX, axisY; // Using smart pointers for axes
  double y[101];
  double x[101];
  int xpos;
  int nvalid; // number of samples received so far (max 101

  // Replaced global variables with class members
  M2550_access* m_instrument;  // Instrument communication object - raw pointer as it's a QObject
  QString m_interface;         // Serial interface name
  QTimer* m_timer;             // Timer for measurements - already handled properly by Qt parent-child


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
