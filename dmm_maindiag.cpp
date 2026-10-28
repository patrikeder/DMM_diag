#include "dmm_maindiag.h"
#include "dmm_maindiag_priv.h"
#include <math.h>
#include <QDebug>
#include <QRegularExpression>


DMM_MainDiag::DMM_MainDiag(QWidget* parent): QDialog(parent), m_instrument(nullptr), m_timer(nullptr)
{
  ui.setupUi(this);

  // Enter in the port field must not trigger a button (e.g. Quit)
  foreach (QPushButton *pb, findChildren<QPushButton*>())
    pb->setAutoDefault(false);

  m_timer = new QTimer(this); // Pass 'this' as parent for proper cleanup
  updateTimer();

  DMMstate = idle;
  curType = "VOLT";
  ui.lbl_unit->setText(base_unit());

  init_curve();

  chart = new QChart();
  curve1 = new QLineSeries();
  chart->addSeries(curve1);
  chart->legend()->hide();
  axisX = new QValueAxis();
  axisY = new QValueAxis();
  axisX->setRange(0, 100);
  chart->addAxis(axisX, Qt::AlignBottom);
  chart->addAxis(axisY, Qt::AlignLeft);
  curve1->attachAxis(axisX);
  curve1->attachAxis(axisY);
  ui.chv_ValPlot->setChart(chart);
  ui.chv_ValPlot->setRenderHint(QPainter::Antialiasing);
  update_curve();

  ui.lcdRate->display(ui.hS_rate->value());

  m_interface = ui.te_interface->text();

  connect(ui.pb_Connect, SIGNAL( clicked() ),
          this, SLOT( slotConnectClicked() ) );

  signalMapperMeas= new QSignalMapper(this);
  signalMapperMeas->setMapping(ui.pb_Meas,waitformeas);
  signalMapperMeas->setMapping(ui.pb_MeasCont,cont_meas);

  connect(ui.pb_Meas, SIGNAL( clicked() ),signalMapperMeas, SLOT( map() ) );
  connect(ui.pb_MeasCont, SIGNAL( clicked() ),signalMapperMeas, SLOT( map() ) );

  connect(signalMapperMeas, SIGNAL(mappedInt(int)),this, SLOT(getMeasurement(int)));


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
  connect(signalMapperMT, SIGNAL(mappedString(QString)), this, SLOT(setMeasurementType(QString)));



  signalMapperDCAC = new QSignalMapper(this);
  signalMapperDCAC->setMapping(ui.rb_DC, QString("DC"));
  signalMapperDCAC->setMapping(ui.rb_AC, QString("AC"));
  connect(ui.rb_DC, SIGNAL(clicked()),signalMapperDCAC, SLOT (map()));
  connect(ui.rb_AC, SIGNAL(clicked()),signalMapperDCAC, SLOT (map()));
  connect(signalMapperDCAC, SIGNAL(mappedString(QString)),this, SLOT(setDCAC(QString)));


  signalMapperRE = new QSignalMapper(this);
  signalMapperRE->setMapping(ui.rb_res_3, 3);
  signalMapperRE->setMapping(ui.rb_res_4, 4);
  signalMapperRE->setMapping(ui.rb_res_5, 5);
  signalMapperRE->setMapping(ui.rb_res_6, 6);
  connect(ui.rb_res_3, SIGNAL(clicked()), signalMapperRE, SLOT (map()));
  connect(ui.rb_res_4, SIGNAL(clicked()),signalMapperRE, SLOT (map()));
  connect(ui.rb_res_5, SIGNAL(clicked()),signalMapperRE, SLOT (map()));
  connect(ui.rb_res_6, SIGNAL(clicked()),signalMapperRE, SLOT (map()));
  connect(signalMapperRE, SIGNAL(mappedInt(int)),this, SLOT(setResolution(int)));


  connect(ui.hS_rate, SIGNAL(valueChanged(int)),this,SLOT(updateTimer()));

  connect(ui.pb_ID,SIGNAL(clicked(bool)),this,SLOT(getID()));
  connect(ui.pb_Settings,SIGNAL(clicked(bool)),this,SLOT(getSettings()));

  connect(ui.pbDisp,SIGNAL(clicked(bool)),this,SLOT(Disp_off()));

  connect(m_timer, SIGNAL(timeout()),this,SLOT(timer_task()));

  m_timer->start();
  this->show();
}

void DMM_MainDiag::init_curve(){
  for ( int pos = 0; pos < 101; pos++ ) {
      x[pos] = pos;
      y[pos] = 0;
    }
  xpos = 0;
  nvalid = 0;
}

void DMM_MainDiag::update_curve(){
  QList<QPointF> points;
  double ymin = 0, ymax = 0;
  for ( int pos = 0; pos < nvalid; pos++ ) {
      points.append(QPointF(x[pos], y[pos]));
      if (pos == 0 || y[pos] < ymin) ymin = y[pos];
      if (pos == 0 || y[pos] > ymax) ymax = y[pos];
    }
  curve1->replace(points);
  // autoscale y, keep a visible span for a flat line or no data
  double margin = (ymax > ymin) ? 0.05*(ymax-ymin) : 1.0;
  axisY->setRange(ymin-margin, ymax+margin);
}

DMM_MainDiag::~DMM_MainDiag()
{
  // no further measurement requests while shutting down
  m_timer->stop();
  DMMstate = idle;
  // null the instrument first: deleting sends the instrument back to local mode, and its
  // answer must not reach slots of this half-destroyed dialog
  M2550_access *instr = m_instrument;
  m_instrument = 0;
  delete instr;
  delete m_timer;
  m_timer = 0;
}


void DMM_MainDiag::slotConnectClicked() {
  if (ui.pb_Connect->isChecked())
    {
      m_interface = ui.cb_sim->isChecked() ? Serial_Access::SIM_INTERFACE : ui.te_interface->text();
      if (m_instrument == NULL) {
          Instr_init();
        }
      else
        {
          m_instrument->M2550_connect(m_interface);
        }
      if (m_instrument->isConnected()){
          QObject::connect(m_instrument,SIGNAL(M2550_ack_received()),this,SLOT(updateMSG()));

        }
    }
  else
    {
      if (m_instrument != NULL) {
          if (m_instrument->isConnected()) {
              m_instrument->disconnect();
            }
        }
    }
  ui.pb_ID->setEnabled(m_instrument->isConnected());
  ui.pb_Settings->setEnabled(m_instrument->isConnected());
  ui.pbDisp->setEnabled(m_instrument->isConnected());

  ui.pb_Meas->setEnabled(m_instrument->isConnected());
  ui.pb_MeasCont->setEnabled(m_instrument->isConnected());

  ui.gB_currty->setEnabled(m_instrument->isConnected());
  ui.rb_AC->setEnabled(m_instrument->isConnected());
  ui.rb_DC->setEnabled(m_instrument->isConnected());

  ui.gb_Meas->setEnabled(m_instrument->isConnected());
  ui.rB_Ampere->setEnabled(m_instrument->isConnected());
  ui.rB_Farad->setEnabled(m_instrument->isConnected());
  ui.rB_Herz->setEnabled(m_instrument->isConnected());
  ui.rB_Ohm->setEnabled(m_instrument->isConnected());
  ui.rB_Volt->setEnabled(m_instrument->isConnected());

  ui.gb_urate->setEnabled(m_instrument->isConnected());
  ui.hS_rate->setEnabled(m_instrument->isConnected());

  // button and port selection follow the real connection state (also if connecting failed)
  bool connected = m_instrument->isConnected();
  ui.pb_Connect->setChecked(connected);
  ui.pb_Connect->setText(connected ? "Disconnect" : "Connect");
  ui.te_interface->setEnabled(!connected);
  ui.cb_sim->setEnabled(!connected);
}

void DMM_MainDiag::updateDBG() {
  // get value
  if (m_instrument != NULL) {
      if (m_instrument->sl_dbg_msg_m2550.size()>0) {
          QString msgs_d = m_instrument->sl_dbg_msg_m2550.join("\n");
          ui.te_output_dbg->append(msgs_d);
          m_instrument->sl_dbg_msg_m2550.clear();
        }
      if (m_instrument->sl_err_msg_m2550.size()>0) {
          QString msgs_e = m_instrument->sl_err_msg_m2550.join("\n");
          ui.te_output_err->append(msgs_e);
          m_instrument->sl_err_msg_m2550.clear();
        }
    }
}

void DMM_MainDiag::updateMSG() {
  if (m_instrument != NULL) {
      if (m_instrument->sl_msg_m2550.size()>0) {
          msg = m_instrument->sl_msg_m2550;
          m_instrument->sl_msg_m2550.clear();
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
  // the reading is the first line that is a number; the YES/NO acknowledge, an echoed
  // command or the stray CR/LF the instrument sends after its acknowledge are skipped
  QString ql_val;
  double val_read = 0;
  foreach (const QString &line, msg.split("\n")){
      bool ok;
      double d = line.trimmed().toDouble(&ok);
      if (ok){
          ql_val = line.trimmed();
          val_read = d;
          break;
        }
    }
  bool overload = ql_val.isEmpty() && msg.contains("OVLD");
  msg.clear();
  if (overload){
      // out of range (e.g. open circuit in ohms mode): no value, so keep it out of the plot
      ui.lcdValue->setDigitCount(5);
      ui.lcdValue->display("  oL"); // lower case: an upper case O would look like a 0
      ui.lbl_unit->setText(base_unit());
      ui.pB_Pos->setValue(0);
      ui.pB_neg->setValue(0);
      return;
    }
  if (ql_val.isEmpty())
    return; // no reading in this reply

  show_value(ql_val, val_read);
  y[xpos] = val_read;
  nvalid = qMax(nvalid, xpos+1);
  update_curve();
  if (xpos>=1){
      double delta = y[xpos-1]-y[xpos]; // difference to previous value
      double val = 10*log(delta/y[xpos]); // scale logarithmically

      if (qAbs(val) < 5 || isnan(val)){  // is log(difference) is to low or not a number
          ui.pB_Pos->setValue(1);
          ui.pB_Pos->setValue(1);
        }
      else{
          if (val>0){
              ui.pB_Pos->setValue((int)qAbs(val));
              ui.pB_neg->setValue(0);
            }
          else{
              ui.pB_Pos->setValue(0);
              ui.pB_neg->setValue((int)qAbs(val));
            }
        }

    }
  xpos++;if(xpos>=101){xpos=0;} //set to 0 once 101 values are dsiplayed
}

QString DMM_MainDiag::base_unit() const{
  if (curType == "VOLT") return "V";
  if (curType == "CURR") return "A";
  if (curType == "RESI") return QString(QChar(0x3A9)); // ohm
  if (curType == "CAP")  return "F";
  if (curType == "FREQ") return "Hz";
  return QString();
}

// Show a reading with exactly the digits the instrument sent, scaled by the instrument's
// exponent to an SI prefix: "+012.345E+6" -> 12.345 M(ohm), "+008.184E-3" -> 8.184 mV.
// The LCD is sized to fit (sign + all digits), as the sign uses up a cell too.
void DMM_MainDiag::show_value(const QString &raw, double val){
  int ei = raw.indexOf('E', 0, Qt::CaseInsensitive);
  QString mant = (ei >= 0) ? raw.left(ei) : raw;
  int e = (ei >= 0) ? raw.mid(ei+1).toInt() : 0;
  int dot = mant.indexOf('.');
  int d = (dot >= 0) ? mant.length()-dot-1 : 0; // decimals of the mantissa

  QString prefix, txt;
  bool scaled = true;
  switch (e){
    case -12: prefix = "p"; break;
    case -9:  prefix = "n"; break;
    case -6:  prefix = QString(QChar(0xB5)); break; // micro
    case -3:  prefix = "m"; break;
    case 0:   break;
    case 3:   prefix = "k"; break;
    case 6:   prefix = "M"; break;
    case 9:   prefix = "G"; break;
    default:  scaled = false; break;
    }

  if (scaled){
      txt = QString::number(mant.toDouble(), 'f', d);
    }
  else{
      // exponent without an SI prefix: plain digits, scientific when too long
      txt = QString::number(val, 'f', qMax(0, d-e));
      if (txt.count(QRegularExpression("[0-9]")) > MAX_LCD_DIGITS){
          int mdigits = mant.count(QRegularExpression("[0-9]"));
          txt = QString::number(val, 'E', qMax(0, mdigits-1));
        }
    }

  int digits = txt.length() - txt.count('.') - txt.count('-');
  ui.lcdValue->setDigitCount(qMax(5, digits+1)); // +1: sign
  ui.lcdValue->display(txt);
  ui.lbl_unit->setText(prefix + base_unit());
}

void DMM_MainDiag::Disp_off(){
  m_instrument->DisplayOFF();
}


void DMM_MainDiag::Instr_init()
{
  m_instrument = new M2550_access(m_interface);
}

void DMM_MainDiag::getID()
{
    m_instrument->getIDN();
}

void DMM_MainDiag::getSettings(){
    m_instrument->getSettings();
}

void DMM_MainDiag::getMeasurement(int state){
  qDebug()<<"MEAS triggered";
  m_instrument->getMeasurement();
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
  m_instrument->setMeasurement(type);
  curType = type;
  ui.lbl_unit->setText(base_unit());
  init_curve();
  update_curve();
}

void DMM_MainDiag::setResolution(int res){
  m_instrument->setResolution(res);
}

void DMM_MainDiag::setDCAC(QString DCAC){
  m_instrument->setDCAC(DCAC);
  init_curve();
  update_curve();
}

void DMM_MainDiag::updateTimer()
{
  m_timer->setInterval(ui.hS_rate->value());
  ui.lcdRate->display(ui.hS_rate->value()); // interval in ms
}

void DMM_MainDiag::timer_task()
{
  if (m_instrument != NULL) {
      updateDBG();
      if (DMMstate == cont_meas){
          m_instrument->getMeasurement();
        }
    }
}

