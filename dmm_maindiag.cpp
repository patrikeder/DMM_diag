#include "dmm_maindiag.h"
#include "dmm_maindiag_priv.h"



DMM_MainDiag::DMM_MainDiag(QWidget* parent): QDialog(parent)
{
    ui.setupUi(this);

    diag_timer = new QTimer();
    diag_timer->setInterval(200);
    //updateTimer();

    ui.lcdRate->display(ui.hS_rate->value());

    cp_interface = ui.te_interface->toPlainText();

    connect( ui.pb_Connect, SIGNAL( clicked() ),
             this, SLOT( slotConnectClicked() ) );

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
        cp_interface = ui.te_interface->toPlainText();
        if (M51_instr == NULL) {
            Instr_init();
        }
        else
        {
            M51_instr->connect(cp_interface);
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
    updateTE();
}

void DMM_MainDiag::updateTE() {
// get value
    if (M51_instr != NULL) {
        if (M51_instr->sl_dbg_msg_m2550.size()>0) {
            QString msgs = M51_instr->sl_dbg_msg_m2550.join("\n");
            ui.te_output_dbg->append(msgs);
            M51_instr->sl_dbg_msg_m2550.clear();
        }
        if (M51_instr->sl_err_msg_m2550.size()>0) {
            QString msgs = M51_instr->sl_err_msg_m2550.join("\n");
            ui.te_output_err->append(msgs);
            M51_instr->sl_err_msg_m2550.clear();
        }
    }
}

void DMM_MainDiag::Instr_init()
{
    M51_instr = new M2550_access(cp_interface);
}

void DMM_MainDiag::getID()
{
    ui.te_output_dbg->append("IDN = "+M51_instr->getIDN());
    updateTE();
}


void DMM_MainDiag::updateTimer()
{
    diag_timer->setInterval(ui.hS_rate->value()*1000);
}

void DMM_MainDiag::timer_task()
{
    updateTE();
    if (M51_instr != NULL) {
            ui.pb_ID->setEnabled(M51_instr->isConnected());        
    }


    if (ui.pB_Pos->value() > 99) {
        ui.pB_Pos->setValue(0);
    }
    ui.pB_Pos->setValue(ui.pB_Pos->value()+1);
}
