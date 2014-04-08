#include "dmm_maindiag.h"
#include "dmm_maindiag_priv.h"


DMM_MainDiag::DMM_MainDiag(QWidget* parent): QDialog(parent)
{
    ui.setupUi(this);

    diag_timer = new QTimer();
    diag_timer->setInterval(200);
    //updateTimer();
    waitformeas = false;

    ui.lcdRate->display(ui.hS_rate->value());

    cp_interface = ui.te_interface->text();

    connect(ui.pb_Connect, SIGNAL( clicked() ),
            this, SLOT( slotConnectClicked() ) );

    connect(ui.pb_Meas, SIGNAL( clicked() ),
            this, SLOT( getMeasurement() ) );


    signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(ui.rB_Volt, QString("VOLT"));
    signalMapper->setMapping(ui.rB_Ampere, QString("CURR"));
    signalMapper->setMapping(ui.rB_Ohm, QString("RESI"));
    signalMapper->setMapping(ui.rB_Farad, QString("CAP"));
    signalMapper->setMapping(ui.rB_Herz, QString("FREQ"));

    connect(ui.rB_Volt, SIGNAL(clicked()),
            signalMapper, SLOT (map()));
    connect(ui.rB_Ampere, SIGNAL(clicked()),
            signalMapper, SLOT (map()));
    connect(ui.rB_Farad, SIGNAL(clicked()),
            signalMapper, SLOT (map()));
    connect(ui.rB_Herz, SIGNAL(clicked()),
            signalMapper, SLOT (map()));
    connect(ui.rB_Ohm, SIGNAL(clicked()),
            signalMapper, SLOT (map()));


    connect(signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(setMeasurementType(QString)));



    connect(ui.hS_rate, SIGNAL(valueChanged(int)),
            this,SLOT(updateTimer()));
    
    connect(ui.pb_ID,SIGNAL(clicked(bool)),
            this,SLOT(getID()));

    connect(diag_timer, SIGNAL(timeout()),
            this,SLOT(timer_task()));

    diag_timer->start();
    this->show();
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
    ui.pb_Meas->setEnabled(M51_instr->isConnected());
    ui.rB_Ampere->setEnabled(M51_instr->isConnected());
    ui.rB_Farad->setEnabled(M51_instr->isConnected());
    ui.rB_Herz->setEnabled(M51_instr->isConnected());
    ui.rB_Ohm->setEnabled(M51_instr->isConnected());
    ui.rB_Volt->setEnabled(M51_instr->isConnected());
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
            ui.te_output_msg->append(msg);
            M51_instr->sl_msg_m2550.clear();
        }
    }
}

void DMM_MainDiag::Instr_init()
{
    M51_instr = new M2550_access(cp_interface);
}

void DMM_MainDiag::getID()
{
    ui.te_output_dbg->append("IDN = ");
    M51_instr->getIDN();
}

void DMM_MainDiag::getMeasurement(){
    qDebug()<<"MEAS triggered";
    M51_instr->getMeasurement();
    waitformeas = true;
}

void DMM_MainDiag::setMeasurementType(QString type){
    M51_instr->setMeasurement(type,"DC");
}

void DMM_MainDiag::updateTimer()
{
    diag_timer->setInterval(ui.hS_rate->value()*1000);
}

void DMM_MainDiag::timer_task()
{
    if (M51_instr != NULL) {
        updateDBG();
    }
    if (waitformeas){
        if(msg.contains("YES")){
            qDebug()<<"VALUE:"<<msg;
            waitformeas=false;
            msg.clear();
        }
        else{
            qDebug()<<"NACK:"<<msg;
            waitformeas=false;
            msg.clear();
        }
    }
}

