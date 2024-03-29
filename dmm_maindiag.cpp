#include "dmm_maindiag.h"
#include "dmm_maindiag_priv.h"
#include <math.h>
#include <QDebug>


DMM_MainDiag::DMM_MainDiag(QWidget* parent): QDialog(parent)
{
  ui.setupUi(this);

  diag_timer = new QTimer();
  updateTimer();

  DMMstate = idle;

  init_curve();

  curve1  = new QwtPlotCurve("Data");

  ui.qwt_ValPlot->setAxisAutoScale(QwtPlot::yRight);
  ui.qwt_ValPlot->setAxisAutoScale(QwtPlot::xTop);
  ui.qwt_ValPlot->setAutoReplot();
  curve1->attach(ui.qwt_ValPlot);
  curve1->setRawSamples(x,y,101);

  ui.lcdRate->display(ui.hS_rate->value());

  cp_interface = ui.te_interface->text();

  connect(ui.pb_Connect, SIGNAL( clicked() ),
          this, SLOT( slotConnectClicked() ) );

  signalMapperMeas= new QSignalMapper(this);
  signalMapperMeas->setMapping(ui.pb_Meas,waitformeas);
  signalMapperMeas->setMapping(ui.pb_MeasCont,cont_meas);

  connect(ui.pb_Meas, SIGNAL( clicked() ),signalMapperMeas, SLOT( map() ) );
  connect(ui.pb_MeasCont, SIGNAL( clicked() ),signalMapperMeas, SLOT( map() ) );

  connect(signalMapperMeas, SIGNAL(mapped(int)),this, SLOT(getMeasurement(int)));


  signalMapperMT = new QSignalMapper(this);
  signalMapperMT->setMapping(ui.rB_Volt, QString("VOLT"));
  signalMapperMT->setMapping(ui.rB_Ampere, QString("CURR"));
  signalMapperMT->setMapping(ui.rB_Ohm, QString("RESI"));
  signalMapperMT->setMapping(ui.rB_Farad, QString("CAP"));
  signalMapperMT->setMapping(ui.rB_Herz, QString("FREQ"));
  connect(ui.rB_Volt, SIGNAL(clicked()), signalMapperMT, SLOT (map()));
  connect(ui.rB_Ampere, SIGNAL(clicked()),signalMapperMT, SLOT (map()));
  connect(ui.rB_Farad, SIGNAL(clicked()),signalMapperMT, SLOT (map()));
  connect(ui.rB_Herz, SIGNAL(clicked()), signalMapperMT, SLOT (map()));
  connect(ui.rB_Ohm, SIGNAL(clicked()),  signalMapperMT, SLOT (map()));
  connect(signalMapperMT, SIGNAL(mapped(QString)), this, SLOT(setMeasurementType(QString)));



  signalMapperDCAC = new QSignalMapper(this);
  signalMapperDCAC->setMapping(ui.rb_DC, QString("DC"));
  signalMapperDCAC->setMapping(ui.rb_AC, QString("AC"));
  connect(ui.rb_DC, SIGNAL(clicked()),signalMapperDCAC, SLOT (map()));
  connect(ui.rb_AC, SIGNAL(clicked()),signalMapperDCAC, SLOT (map()));
  connect(signalMapperDCAC, SIGNAL(mapped(QString)),this, SLOT(setDCAC(QString)));


  signalMapperRE = new QSignalMapper(this);
  signalMapperRE->setMapping(ui.rb_res_3, 3);
  signalMapperRE->setMapping(ui.rb_res_4, 4);
  signalMapperRE->setMapping(ui.rb_res_5, 5);
  signalMapperRE->setMapping(ui.rb_res_6, 6);
  connect(ui.rb_res_3, SIGNAL(clicked()), signalMapperRE, SLOT (map()));
  connect(ui.rb_res_4, SIGNAL(clicked()),signalMapperRE, SLOT (map()));
  connect(ui.rb_res_5, SIGNAL(clicked()),signalMapperRE, SLOT (map()));
  connect(ui.rb_res_6, SIGNAL(clicked()),signalMapperRE, SLOT (map()));
  connect(signalMapperRE, SIGNAL(mapped(int)),this, SLOT(setResolution(int)));


  connect(ui.hS_rate, SIGNAL(valueChanged(int)),this,SLOT(updateTimer()));

  connect(ui.pb_ID,SIGNAL(clicked(bool)),this,SLOT(getID()));
  connect(ui.pb_Settings,SIGNAL(clicked(bool)),this,SLOT(getSettings()));

  connect(ui.pbDisp,SIGNAL(clicked(bool)),this,SLOT(Disp_off()));

  connect(diag_timer, SIGNAL(timeout()),this,SLOT(timer_task()));

  diag_timer->start();
  this->show();
}

void DMM_MainDiag::init_curve(){
  for ( int pos = 0; pos < 101; pos++ ) {
      x[pos] = pos;
      y[pos] = 0;
    }
  xpos = 0;
}

DMM_MainDiag::~DMM_MainDiag()
{
  M51_instr->~M2550_access();
}


void DMM_MainDiag::slotConnectClicked() {
  if (ui.pb_Connect->isChecked())
    {
      cp_interface = ui.te_interface->text();
      if (M51_instr == NULL) {
          Instr_init();
        }
      else
        {
          M51_instr->M2550_connect(cp_interface);
        }
      if (M51_instr->isConnected()){
          QObject::connect(M51_instr,SIGNAL(M2550_ack_received()),this,SLOT(updateMSG()));

        }
    }
  else
    {
      if (M51_instr != NULL) {
          if (M51_instr->isConnected()) {
              M51_instr->disconnect();
            }
        }
    }
  ui.pb_ID->setEnabled(M51_instr->isConnected());
  ui.pb_Settings->setEnabled(M51_instr->isConnected());
  ui.pbDisp->setEnabled(M51_instr->isConnected());

  ui.pb_Meas->setEnabled(M51_instr->isConnected());
  ui.pb_MeasCont->setEnabled(M51_instr->isConnected());

  ui.gB_currty->setEnabled(M51_instr->isConnected());
  ui.rb_AC->setEnabled(M51_instr->isConnected());
  ui.rb_DC->setEnabled(M51_instr->isConnected());

  ui.gb_Meas->setEnabled(M51_instr->isConnected());
  ui.rB_Ampere->setEnabled(M51_instr->isConnected());
  ui.rB_Farad->setEnabled(M51_instr->isConnected());
  ui.rB_Herz->setEnabled(M51_instr->isConnected());
  ui.rB_Ohm->setEnabled(M51_instr->isConnected());
  ui.rB_Volt->setEnabled(M51_instr->isConnected());

  ui.gb_urate->setEnabled(M51_instr->isConnected());
  ui.hS_rate->setEnabled(M51_instr->isConnected());
}

void DMM_MainDiag::updateDBG() {
  // get value
  if (M51_instr != NULL) {
      if (M51_instr->sl_dbg_msg_m2550.size()>0) {
          QString msgs_d = M51_instr->sl_dbg_msg_m2550.join("\n");
          ui.te_output_dbg->append(msgs_d);
          M51_instr->sl_dbg_msg_m2550.clear();
        }
      if (M51_instr->sl_err_msg_m2550.size()>0) {
          QString msgs_e = M51_instr->sl_err_msg_m2550.join("\n");
          ui.te_output_err->append(msgs_e);
          M51_instr->sl_err_msg_m2550.clear();
        }
    }
}

void DMM_MainDiag::updateMSG() {
  if (M51_instr != NULL) {
      if (M51_instr->sl_msg_m2550.size()>0) {
          msg = M51_instr->sl_msg_m2550;
          M51_instr->sl_msg_m2550.clear();
          // negative relply
          if (msg.contains("NO")){
              ui.te_output_dbg->append("NACK: "+msg);
              msg.clear();
            }
          // something happened
          if (msg.contains("YES")){
              switch (DMMstate){
                // continuous measurement
                case cont_meas:
                  ui.te_output_msg->append(msg);
                  Disp_val();
                  break;
                // single measurement  -> go to idle
                case waitformeas:
                  ui.te_output_msg->append(msg);
                  Disp_val();
                  DMMstate = idle;
                  break;
                // Not requested, just report
                default:
                  ui.te_output_msg->append(msg);
                  msg.clear();
                  break;
                }
            }
        }
    }
}

void DMM_MainDiag::Disp_val(){
  QString ql_val = msg.split("\n").value(1);
  msg.clear();

  ui.lcdValue->display(ql_val.toDouble());
  y[xpos] = ql_val.toDouble();
  curve1->setRawSamples(x,y,101);
  if (xpos>=1){
      double delta = y[xpos-1]-y[xpos]; // difference to previous value
      double val = 10*log(delta/y[xpos]); // scale logarithmically

      if (abs(val) < 5 || isnan(val)){  // is log(difference) is to low or not a number
          ui.pB_Pos->setValue(1);
          ui.pB_Pos->setValue(1);
        }
      else{
          if (val>0){
              ui.pB_Pos->setValue((int)abs(val));
              ui.pB_neg->setValue(0);
            }
          else{
              ui.pB_Pos->setValue(0);
              ui.pB_neg->setValue((int)abs(val));
            }
        }

    }
  xpos++;if(xpos>101){xpos=0;} //set to 0 once 101 values are dsiplayed
}

void DMM_MainDiag::Disp_off(){
  M51_instr->DisplayOFF();
}


void DMM_MainDiag::Instr_init()
{
  M51_instr = new M2550_access(cp_interface);
}

void DMM_MainDiag::getID()
{
    M51_instr->getIDN();
}

void DMM_MainDiag::getSettings(){
    M51_instr->getSettings();
}

void DMM_MainDiag::getMeasurement(int state){
  qDebug()<<"MEAS triggered";
  M51_instr->getMeasurement();
  switch (state){
    case cont_meas:
      switch (DMMstate){
        case idle:
          DMMstate = cont_meas;
          break;
        case cont_meas:
          DMMstate = idle;
        default:
          DMMstate = (eDMMstate)state;
          break;
        }
      break;
    default:
      DMMstate = (eDMMstate)state;
      break;
    }
}

void DMM_MainDiag::setMeasurementType(QString type){
  M51_instr->setMeasurement(type);
  init_curve();
}

void DMM_MainDiag::setResolution(int res){
  M51_instr->setResolution(res);
  ui.lcdValue->setDigitCount(res);
}

void DMM_MainDiag::setDCAC(QString DCAC){
  M51_instr->setDCAC(DCAC);
  init_curve();
}

void DMM_MainDiag::updateTimer()
{
  diag_timer->setInterval(ui.hS_rate->value());
  ui.lcdRate->display(ui.hS_rate->value()/10);
}

void DMM_MainDiag::timer_task()
{
  if (M51_instr != NULL) {
      updateDBG();
      if (DMMstate == cont_meas){
          M51_instr->getMeasurement();
        }
    }
}

