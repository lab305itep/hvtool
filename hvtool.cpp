#include <stdio.h>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include "hvtool.h"
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
    int i;
    const char ADCnames[][15] = {"HV set", "VREF", "GND", "DACTEMP", "HVMV", "LVMV", "HVFB", "HVMI", "CPUTEMP"};
    
    errorCnt = 0;
    
    if(objectName().isEmpty()) setObjectName(QString::fromUtf8("MainWindow"));
    setWindowTitle(QApplication::translate("MainWindow", "HVDAC control tool", 0, QApplication::UnicodeUTF8));
    
    resize(800, 300);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    setSizePolicy(sizePolicy);

    QIcon icon;
    icon.addFile(QString::fromUtf8("pencil_scale.png"), QSize(), QIcon::Normal, QIcon::Off);
    setWindowIcon(icon);
    
    topframe = new QWidget(this);
    topframe->setObjectName(QString::fromUtf8("topframe"));
    verticalLayout = new QVBoxLayout(topframe);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    
    for (i=0; i<3; i++) {
        vlColoumn[i] = new QVBoxLayout();
        vlColoumn[i]->setObjectName(QString("vlColoumn%1").arg(i));
    }
    for (i=0; i<24; i++) {
        hl[i] = new QHBoxLayout();
        hl[i]->setObjectName(QString("hl%1").arg(i));
        hl[i]->setSizeConstraint(QLayout::SetDefaultConstraint);
        hl[i]->setContentsMargins(0, 0, 0, -1);
        
        lb[i] = new QLabel(topframe);
        lb[i]->setObjectName(QString("lb%1").arg(i));
        if (i < 15) {
            lb[i]->setText(QString("Chan %1").arg(i, 2));
        } else {
            lb[i]->setText(ADCnames[i - 15]);
        }
        hl[i]->addWidget(lb[i]);

        hsp[i] = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        hl[i]->addItem(hsp[i]);

        bxRead[i] = new QDoubleSpinBox(topframe);
        bxRead[i]->setObjectName(QString("bxRead%1").arg(i));
        bxRead[i]->setInputMethodHints(Qt::ImhNone);
        bxRead[i]->setReadOnly(true);
        bxRead[i]->setRange(-100, 100);
        bxRead[i]->setButtonSymbols(QAbstractSpinBox::NoButtons);
        hl[i]->addWidget(bxRead[i]);
        
        if (i < 16) {

            bnOn[i] = new QPushButton(topframe);
            bnOn[i]->setObjectName(QString("bnOn%1").arg(i));
            sizePolicy.setHeightForWidth(bnOn[i]->sizePolicy().hasHeightForWidth());
            bnOn[i]->setSizePolicy(sizePolicy);
            bnOn[i]->setCheckable(true);
            bnOn[i]->setText(QApplication::translate("MainWindow", "On", 0, QApplication::UnicodeUTF8));
            hl[i]->addWidget(bnOn[i]);

            bxWrite[i] = new QDoubleSpinBox(topframe);
            bxWrite[i]->setObjectName(QString("bxWrite%1").arg(i));
            bxWrite[i]->setMinimum(-10);
            bxWrite[i]->setMaximum(10);
            bxWrite[i]->setSingleStep(0.1);
            bxWrite[i]->setValue((i == 15) ? -10 : 10);
            hl[i]->addWidget(bxWrite[i]);

            bnSet[i] = new QCommandLinkButton(topframe);
            bnSet[i]->setObjectName(QString("bnSet%1").arg(i));
            sizePolicy.setHeightForWidth(bnSet[i]->sizePolicy().hasHeightForWidth());
            bnSet[i]->setSizePolicy(sizePolicy);
            bnSet[i]->setText(QApplication::translate("MainWindow", "Set", 0, QApplication::UnicodeUTF8));
            hl[i]->addWidget(bnSet[i]);
        }
        
        vlColoumn[i/8]->addLayout(hl[i]);
    }
    
    for (i=0; i<3; i++) horizontalLayout->addLayout(vlColoumn[i]);

    verticalLayout->addLayout(horizontalLayout);

    verticalSpacer = new QSpacerItem(20, 562, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(verticalSpacer);

    hlBottom = new QHBoxLayout();
    hlBottom->setObjectName(QString::fromUtf8("hlBottom"));
    hlBottom->setSizeConstraint(QLayout::SetDefaultConstraint);

    lbLV = new QLabel(topframe);
    lbLV->setObjectName(QString::fromUtf8("lbLV"));
    lbLV->setText(QApplication::translate("MainWindow", "Preamp power", 0, QApplication::UnicodeUTF8));
    hlBottom->addWidget(lbLV);

    bnLV = new QPushButton(topframe);
    bnLV->setObjectName(QString::fromUtf8("bnLV"));
    bnLV->setCheckable(true);
    bnLV->setText(QApplication::translate("MainWindow", "On", 0, QApplication::UnicodeUTF8));
    hlBottom->addWidget(bnLV);

    hspBottom = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlBottom->addItem(hspBottom);

	lb1WTemp = new QLabel(topframe);
    lb1WTemp->setObjectName(QString::fromUtf8("lb1WTemp"));
    lb1WTemp->setText(QApplication::translate("MainWindow", "1Wire temperature", 0, QApplication::UnicodeUTF8));
    hlBottom->addWidget(lb1WTemp);

    bx1WTemp = new QDoubleSpinBox(topframe);
    bx1WTemp->setObjectName("bx1WTemp");
    bx1WTemp->setInputMethodHints(Qt::ImhNone);
    bx1WTemp->setReadOnly(true);
    bx1WTemp->setRange(-100, 100);
    bx1WTemp->setButtonSymbols(QAbstractSpinBox::NoButtons);
    hlBottom->addWidget(bx1WTemp);

    lbLed = new QLabel(topframe);
    lbLed->setObjectName(QString::fromUtf8("lbLed"));
    lbLed->setText(QApplication::translate("MainWindow", "Communication error:", 0, QApplication::UnicodeUTF8));
    hlBottom->addWidget(lbLed);

    led = new ColorLed(topframe, Qt::black, 0.8, true);
    led->setObjectName(QString::fromUtf8("led"));
    hlBottom->addWidget(led);

    lbAddress = new QLabel(topframe);
    lbAddress->setObjectName(QString::fromUtf8("lbAddress"));
    lbAddress->setText(QApplication::translate("MainWindow", "Unit Address", 0, QApplication::UnicodeUTF8));
    hlBottom->addWidget(lbAddress);

    bxAddress = new QSpinBox(topframe);
    bxAddress->setObjectName(QString::fromUtf8("bxAddress"));
    bxAddress->setMinimum(0);
    bxAddress->setMaximum(255);
    bxAddress->setValue(addr);
    hlBottom->addWidget(bxAddress);

    bnExit = new QPushButton(topframe);
    bnExit->setObjectName(QString::fromUtf8("bnExit"));
    sizePolicy.setHeightForWidth(bnExit->sizePolicy().hasHeightForWidth());
    bnExit->setSizePolicy(sizePolicy);
    bnExit->setAutoDefault(false);
    bnExit->setText(QApplication::translate("MainWindow", "Exit", 0, QApplication::UnicodeUTF8));
    hlBottom->addWidget(bnExit);

    verticalLayout->addLayout(hlBottom);
    setCentralWidget(topframe);

    on_bxAddress_valueChanged(bxAddress->value());    
    
    connect(bnExit, SIGNAL(clicked()), this, SLOT(close()));
    for (i=0; i<16; i++) {
        connect(bnOn[i], SIGNAL(clicked(bool)), this, SLOT(bnOn_clicked(bool)));
        connect(bnOn[i], SIGNAL(toggled(bool)), this, SLOT(bnOn_toggled(bool)));
        connect(bnSet[i], SIGNAL(clicked()), this, SLOT(bnSet_clicked()));
    }
    QMetaObject::connectSlotsByName(this);
    
    timer = new QTimer(this);
    timer->setObjectName(QString::fromUtf8("timer"));
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(500);      // ms
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
    bnLV->setText(QApplication::translate("MainWindow", (checked) ? "Off" : "On", 0, QApplication::UnicodeUTF8));
}

void MainWindow::bnOn_clicked(bool checked)
{
    int i;
    for (i=0; i<16; i++) if (sender() == bnOn[i]) break;
    if (i < 15) {
        if (checked) {
            if (SwitchOn(bxAddress->value(), i) < 0) errorCnt++;
        } else {
            if (SwitchOff(bxAddress->value(), i) < 0) errorCnt++;
        }        
    } else if (i == 15) {
        if (checked) {
            if (HVOn(bxAddress->value()) < 0) errorCnt++;
        } else {
            if (HVOff(bxAddress->value()) < 0) errorCnt++;
        }
    }
}

void MainWindow::bnOn_toggled(bool checked)
{
    ((QPushButton *)sender())->setText(QApplication::translate("MainWindow", (checked) ? "Off" : "On", 0, QApplication::UnicodeUTF8));
}

void MainWindow::bnSet_clicked(void)
{
    int i, val;
    for (i=0; i<16; i++) if (sender() == bnSet[i]) break;
    if (i < 16) {
        val = 3276.8 * bxWrite[i]->value();
        if (val >= 32768) val = 32767;
        if (val < -32768) val = -32768;
        if (WriteDAC(bxAddress->value(), i, val) < 0) errorCnt++;
    }
}

void MainWindow::on_bxAddress_valueChanged(int num)
{
    int i, val;
    double dval;
    errorCnt = 0;
    for (i=0; i<16; i++) {
        val = ReadDAC(num, i);
        if (val < 0) {
            errorCnt++;
        } else {
            if (val & 0x8000) val |= 0xFFFF0000;    // extend sign
            dval = val / 3276.8;
            bxWrite[i]->setValue(dval);
        }
    }
}

void MainWindow::onTimer(void)
{
    int i, val;
    double dval;
    unsigned int uval;
    bool b;
    QColor ldc;
    errorCnt = 0;
    for(i=0; i<24; i++) {
        val = ReadADC(bxAddress->value(), i);   // integer -32768..+32767
        if (val < 0) {
            errorCnt++;
            continue;        // error
        }
        dval = val / 65536.0 * 2.5;   // double  0..2.5
        switch (i) {
        case ADC_TDAC:
            dval = 25 + (dval - 1.46)/0.0044;   // from manual : 1.46 V @ 25C and 4.4 mV/C
            break;
        case ADC_HVMV:
            dval *= 97.6 / 3.6;                 // HV resistor divider
            break;
        case ADC_PAMV:
            dval = 2*dval - 5.0;                // -4V
            break;
        case ADC_HVFB:                          // no translation needed
//            printf("HWFB: %5d", val);
            break;
        case ADC_HVMI:
//            printf("    HVMI: %5d", val);
            dval *= 5 / 0.412;                  // uA: 412k and 5x
            break;
        case ADC_TADC:
//            printf("    TADC: %5d\n", val, dval);
            dval = (dval - 0.0543) / 0.000205;  // from manual : 54.3 mV @ 0C and 205 uV/C
            break;
        default:
            dval = (val - 0x8000) / 3276.8;
            break;
        }
        bxRead[i]->setValue(dval);
    }

    val = ReadADC(bxAddress->value(), ADC_T1W);   // integer -32768..+32767
    if (val < 0) {
		errorCnt++;
	} else {
		if (val & 0x8000) val |= 0xFFFF0000;
		bx1WTemp->setValue(val / 16.0);
	}
    
    uval = GetStatus(bxAddress->value());
    if (uval != 0xFFFFFFFF) {
        b = ON_LV & uval;
        if (b != bnLV->isChecked()) bnLV->setChecked(b);
        b = ON_HV & uval;
        if (b != bnOn[15]->isChecked()) bnOn[15]->setChecked(b);    
        for (i=0; i<15; i++) {
            b = (1 << i + 16) & uval;
            if (b != bnOn[i]->isChecked()) bnOn[i]->setChecked(b);
        }
    } else {
        errorCnt++;
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
    dev = (argc > 1) ? argv[1] : (char *)"/dev/ttyS0";
    addr = (argc > 2) ? (strtol(argv[2], NULL, 0) & 0xFF) : 15;
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
