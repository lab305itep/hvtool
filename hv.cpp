/*
		SvirLex 2014, ITEP, Moscow
	SiPM HV test application. Simple HV control.
	Run as: ./hv <tty device> <initial board address>
	<tty device> is like /dev/ttyS0 for the first PC COM port or /dev/ttyUSB0 etc.
*/

#include <stdio.h>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include "hv.h"
#include "hvop.h"
#include "cmddef.h"

/************************
 *  class ColorLed      *
 ************************/
ColorLed::ColorLed(QWidget *parent, const QColor &cl, const double rs, const bool rnd) : 
    QPushButton(parent), myColor(cl), relSize(rs), isRound(rnd)
{
    setEnabled(false);
}

void ColorLed::paintEvent(QPaintEvent * event)
{
    int w, h;
    

    QPainter painter(this);
    w = event->rect().width()  * (1 - relSize) / 2;
    h = event->rect().height() * (1 - relSize) / 2;
    QRect rc = event->rect().adjusted(w, h, -w, -h);
    painter.setBrush(QBrush(myColor));

    if (isRound) {
        painter.drawEllipse(rc);
    } else {    
        painter.fillRect(rc, myColor);
    }
}

/************************
 *  class MainWindow    *
 ************************/
MainWindow::MainWindow(unsigned char addr, QWidget *parent) : QMainWindow(parent)
{
    int i, j;
    QHBoxLayout *hL;
    QVBoxLayout *hC;
    QLabel *lb;
    errorCnt = 0;
    HVset = 10;
    
    if(objectName().isEmpty()) setObjectName(QString::fromUtf8("MainWindow"));
    setWindowTitle(QApplication::translate("MainWindow", "HVDAC control tool (user version)", 0, QApplication::UnicodeUTF8));
    
    QIcon icon;
    icon.addFile("pencil_scale.png", QSize(), QIcon::Normal, QIcon::Off);
    setWindowIcon(icon);
    
    QWidget *topframe = new QWidget(this);
    topframe->setObjectName("topframe");

    QVBoxLayout *vL = new QVBoxLayout(topframe);
    vL->setObjectName("verticalLayout");
//      Preamp power ON    
    hL = new QHBoxLayout();
    hL->setObjectName("hL_PreampPower");
    lb = new QLabel(topframe);
    lb->setObjectName("lb_PreampPower");
    lb->setText("Preamplifire power");
    hL->addWidget(lb);
    hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    bnLV = new QPushButton(topframe);
    bnLV->setObjectName("bnLV");
    bnLV->setCheckable(true);
    bnLV->setText("On");
    hL->addWidget(bnLV);
    vL->addLayout(hL);
//      HV base power
    hL = new QHBoxLayout();
    hL->setObjectName("hL_HVPower");
    lb = new QLabel(topframe);
    lb->setObjectName("lb_HVPower");
    lb->setText("Base HV");
    hL->addWidget(lb);
    hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    bxHVWrite = new QDoubleSpinBox(topframe);
    bxHVWrite->setObjectName("bxHVWrite");
    bxHVWrite->setMinimum(10);
    bxHVWrite->setMaximum(65);
    bxHVWrite->setSingleStep(0.1);
    bxHVWrite->setValue(HVset);
    bxHVWrite->setSuffix(" V");
    hL->addWidget(bxHVWrite);
    bnHV = new QPushButton(topframe);
    bnHV->setObjectName("bnHV");
    bnHV->setCheckable(true);
    bnHV->setText("On");
    hL->addWidget(bnHV);
    bxHVRead = new QDoubleSpinBox(topframe);
    bxHVRead->setObjectName("bxHVRead");
    bxHVRead->setInputMethodHints(Qt::ImhNone);
    bxHVRead->setReadOnly(true);
    bxHVRead->setRange(0, 100);
    bxHVRead->setButtonSymbols(QAbstractSpinBox::NoButtons);
    bxHVRead->setSuffix(" V");
    hL->addWidget(bxHVRead);
    vL->addLayout(hL);
//      Bias current
    hL = new QHBoxLayout();
    hL->setObjectName("hL_BiasCurrent");
    lb = new QLabel(topframe);
    lb->setObjectName("lb_BiasCurrent");
    lb->setText("SiPM bias current (zero current approximately subtracted)");
    hL->addWidget(lb);
    hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    bxIRead = new QDoubleSpinBox(topframe);
    bxIRead->setObjectName("bxIRead");
    bxIRead->setInputMethodHints(Qt::ImhNone);
    bxIRead->setReadOnly(true);
    bxIRead->setRange(0, 100);
    bxIRead->setButtonSymbols(QAbstractSpinBox::NoButtons);
    bxIRead->setSuffix(" uA");
    hL->addWidget(bxIRead);
    vL->addLayout(hL);
//      Comment about HV
    lb = new QLabel(topframe);
    lb->setObjectName("lb_HVComment");
    lb->setWordWrap(true);
    lb->setText("Individual SiPM voltages can be set in the range +- 10 V relative to the base voltage setting above. Set and readout show intended and resulting voltage on SiPM.");
    vL->addWidget(lb);
//      Individual voltages in 3 coloumns
    QHBoxLayout *hLL = new QHBoxLayout();
    hLL->setObjectName("hLL");
    for (i=0; i<3; i++) {
        hC = new QVBoxLayout();
        hC->setObjectName(QString("Coloumn%1").arg(i));
        for (j=0; j<5; j++) {
            hL = new QHBoxLayout();
            hL->setObjectName(QString("hL%1").arg(5*i + j));
            lb = new QLabel(topframe);
            lb->setObjectName(QString("lb%1").arg(5*i + j));
            lb->setText(QString("Ch %1").arg(5*i + j + 1));
            hL->addWidget(lb);
            hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
            bxWrite[5*i + j] = new QDoubleSpinBox(topframe);
            bxWrite[5*i + j]->setObjectName(QString("Wr%1").arg(5*i + j));
            bxWrite[5*i + j]->setMinimum(0);
            bxWrite[5*i + j]->setMaximum(20);
            bxWrite[5*i + j]->setSingleStep(0.1);
            bxWrite[5*i + j]->setValue(0);
            bxWrite[5*i + j]->setSuffix(" V");
            hL->addWidget(bxWrite[5*i + j]);
            bnOn[5*i + j] = new QPushButton(topframe);
            bnOn[5*i + j]->setObjectName(QString("bnOn%1").arg(5*i + j));
            bnOn[5*i + j]->setCheckable(true);
            bnOn[5*i + j]->setText("On");
            hL->addWidget(bnOn[5*i + j]);
            bxRead[5*i + j] = new QDoubleSpinBox(topframe);
            bxRead[5*i + j]->setObjectName(QString("Rd%1").arg(5*i + j));
            bxRead[5*i + j]->setInputMethodHints(Qt::ImhNone);
            bxRead[5*i + j]->setReadOnly(true);
            bxRead[5*i + j]->setRange(0, 100);
            bxRead[5*i + j]->setButtonSymbols(QAbstractSpinBox::NoButtons);
            bxRead[5*i + j]->setSuffix(" V");
            hL->addWidget(bxRead[5*i + j]);
            hC->addLayout(hL);
        }
        if (i) hLL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
        hLL->addLayout(hC);
    }    
    vL->addLayout(hLL);
//      Spacer here
    vL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
//      Address and Exit
    hL = new QHBoxLayout();
    hL->setObjectName("hL_Exit");
    lb = new QLabel(topframe);
    lb->setObjectName("lb_Address");
    lb->setText("Board address");
    hL->addWidget(lb);
    bxAddress = new QSpinBox(topframe);
    bxAddress->setObjectName("bxAddress");
    bxAddress->setMinimum(0);
    bxAddress->setMaximum(255);
    bxAddress->setValue(addr);
    hL->addWidget(bxAddress);
    lb = new QLabel(topframe);
    lb->setObjectName("lbLed");
    lb->setText("Communication status");
    hL->addWidget(lb);
    led = new ColorLed(topframe, Qt::black, 0.8, true);
    led->setObjectName("led");
    hL->addWidget(led);
    hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    bnClear = new QPushButton(topframe);
    bnClear->setObjectName("bnClear");
    bnClear->setAutoDefault(false);
    bnClear->setText("Clear");
    hL->addWidget(bnClear);
    hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    bnReset = new QPushButton(topframe);
    bnReset->setObjectName("bnReset");
    bnReset->setAutoDefault(false);
    bnReset->setText("Reset");
    hL->addWidget(bnReset);
    hL->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    bnExit = new QPushButton(topframe);
    bnExit->setObjectName("bnExit");
    bnExit->setAutoDefault(false);
    bnExit->setText("Exit");
    hL->addWidget(bnExit);
    vL->addLayout(hL);
//      Starting
    setCentralWidget(topframe);
    on_bxAddress_valueChanged(bxAddress->value());    
    connect(bnExit, SIGNAL(clicked()), this, SLOT(close()));
    for (i=0; i<15; i++) {
        connect(bxWrite[i], SIGNAL(valueChanged(double)), this, SLOT(bxWriteChanged(double)));
        connect(bnOn[i], SIGNAL(clicked(bool)), this, SLOT(bnOnClicked(bool)));
        connect(bnOn[i], SIGNAL(toggled(bool)), this, SLOT(bnOnToggled(bool)));
    }
    QMetaObject::connectSlotsByName(this);
    
    timer = new QTimer(this);
    timer->setObjectName(QString::fromUtf8("timer"));
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(500);      // ms
}

void MainWindow::bxWriteChanged(double num)
{
    int i;
    int val;
    for (i=0; i<15; i++) if (sender() == bxWrite[i]) break;
    if (i < 15) {    // check for internal error
        val = 3276.8 * (HVset - num);
        if (val >= 32768) val = 32767;
        if (val < -32768) val = -32768;
        if (WriteDAC(bxAddress->value(), i, val) < 0) errorCnt++;
    }
}

void MainWindow::on_bxHVWrite_valueChanged(double num)
{
    int i;
    double val, delta;
    
    delta = num - HVset;
    HVset = num;
    val = 3276.8 * ((num - scoef[0]) / scoef[1]);
    if (val >= 32768) val = 32767;
    if (val < -32768) val = -32768;
    if (WriteDAC(bxAddress->value(), 15, val) < 0) errorCnt++;
    for (i=0; i<15; i++) {
        bxWrite[i]->setMinimum(num - 10);
        bxWrite[i]->setMaximum(num + 10);
        bxWrite[i]->setValue(bxWrite[i]->value() + delta);
    }
}

void MainWindow::on_bnLV_clicked(bool checked)
{
    if (checked) {
        if (LVOn(bxAddress->value()) < 0) errorCnt++;
    } else {
        if (LVOff(bxAddress->value()) < 0) errorCnt++;
    }
}

void MainWindow::on_bnLV_toggled(bool checked)
{
    bnLV->setText((checked) ? "Off" : "On");
}

void MainWindow::on_bnHV_clicked(bool checked)
{
    if (checked) {
        if (HVOn(bxAddress->value()) < 0) errorCnt++;
    } else {
        if (HVOff(bxAddress->value()) < 0) errorCnt++;
    }
}

void MainWindow::on_bnHV_toggled(bool checked)
{
    bnHV->setText((checked) ? "Off" : "On");
}

void MainWindow::on_bnClear_clicked(bool checked)
{
    CmdClear(bxAddress->value());
    CmdSleep(1);
    on_bxAddress_valueChanged(bxAddress->value());
}

void MainWindow::on_bnReset_clicked(bool checked)
{
    CmdReset(bxAddress->value());
    CmdSleep(1);
}

void MainWindow::bnOnClicked(bool checked)
{
    int i;
    for (i=0; i<15; i++) if (sender() == bnOn[i]) break;
    if (i < 15) {
        if (checked) {
            if (SwitchOn(bxAddress->value(), i) < 0) errorCnt++;
        } else {
            if (SwitchOff(bxAddress->value(), i) < 0) errorCnt++;
        }
    }
}

void MainWindow::bnOnToggled(bool checked)
{
    ((QPushButton *)sender())->setText((checked) ? "Off" : "On");
}

void MainWindow::on_bxAddress_valueChanged(int num)
{
    int i, val;
    double dval;

    scoef[0] = 32.00;
    scoef[1] = 3.55;
    rcoef[0] = 0.001 * EEPRead(num, EE_HVCA);
    rcoef[1] = 0.00002 * EEPRead(num, EE_HVCB);
    I0 = 0.7;
    errorCnt = 0;
    
    val = ReadDAC(num, 15); // read HV
    if (val < 0) {
        errorCnt++;
    } else {
        if (val & 0x8000) val |= 0xFFFF0000;    // extend sign
        dval = scoef[0] + (val / 3276.8) * scoef[1];
        HVset = dval;
        bxHVWrite->setValue(dval);
        on_bxHVWrite_valueChanged(dval);
    }
    
    for (i=0; i<15; i++) {  // read others
        val = ReadDAC(num, i);
        if (val < 0) {
            errorCnt++;
        } else {
            if (val & 0x8000) val |= 0xFFFF0000;    // extend sign
            dval = HVset - val / 3276.8;
            bxWrite[i]->setValue(dval);
        }
    }
}

void MainWindow::onTimer(void)
{
    int i, val;
    double dval;
    double HV;
    unsigned int uval;
    bool b;
    QColor ldc;
    errorCnt = 0;

//      Read status
    uval = GetStatus(bxAddress->value());
    if (uval != 0xFFFFFFFF) {
        b = ON_LV & uval;
        if (b != bnLV->isChecked()) bnLV->setChecked(b);
        b = ON_HV & uval;
        if (b != bnHV->isChecked()) bnHV->setChecked(b);    
        for (i=0; i<15; i++) {
            b = (1 << i + 16) & uval;
            if (b != bnOn[i]->isChecked()) bnOn[i]->setChecked(b);
        }
    } else {
        errorCnt++;
    }

//      Read HVMV
    val = ReadADC(bxAddress->value(), ADC_HVMV);   // integer -32768..+32767
    if (val < 0) {
        errorCnt++;
    } else {
        dval = 0.001 * val; 
        // 65536.0 * 2.5;         // double  0..2.5
//        dval *= 97.6 / 3.6;                 // HV resistor divider
        HV = rcoef[0] + dval * rcoef[1];
        bxHVRead->setValue(HV);
    }
//      Read HVMI
    val = ReadADC(bxAddress->value(), ADC_HVMI);   // integer -32768..+32767
    if (val < 0) {
        errorCnt++;
    } else {
        dval = val / 65536.0 * 2.5;         // double  0..2.5
        dval *= 5 / 0.412;                  // uA: 412k and 5x
        bxIRead->setValue(dval - I0);
    }
//      Read DACs for individual channels
    for(i=0; i<15; i++) {
        if (bnHV->isChecked() && bnOn[i]->isChecked()) {
            val = ReadDAC(bxAddress->value(), i);   // we read just DAC setting here
            if (val & 0x8000) val |= 0xFFFF0000;    // extend sign
            dval = HV - val / 3276.8;
        } else {
            dval = HV - 10;
        }
        bxRead[i]->setValue(dval);
    }
    
    switch(errorCnt) {
        case 0:  ldc = Qt::darkGreen;    break;
        case 1:  ldc = Qt::yellow;       break;
        default: ldc = Qt::red;          break;
    }
    led->setColor(ldc);    
}

int main(int argc, char *argv[])
{
    int rc;
    int addr;
    char *dev;
    char msg[1024];

    if (argc > 1 && argv[1][0] != '/') {
        QMessageBox::critical(NULL, "HVDAC control tool", "Usage: hv <tty device> <board address>");        
        return -1;
    }
    dev = (argc > 1) ? argv[1] : (char *)"/dev/ttyS0";
    addr = (argc > 2) ? (strtol(argv[2], NULL, 0) & 0xFF) : 1;
    QApplication app(argc, argv);
    if (DevOpen(dev) < 0) {
	    sprintf(msg, "Can not open the serial port %s. Check permissions etc.", dev);
        QMessageBox::critical(NULL, "HVDAC control tool", msg);
        rc = -1;
    } else {
        MainWindow window(addr);
        window.show();
        rc = app.exec();
        DevClose();
    }
    return rc;
}

