#ifndef HV_H
#define HV_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class ColorLed : public QPushButton
{
    private:
        QColor myColor;     // the LED color
        double relSize;     // LED active area relative size
        bool isRound;       // 0 - rect, 1 - ellipse
    public:
        ColorLed(QWidget *parent = 0, const QColor &cl = Qt::black, const double rs = 1.0, const bool rnd = false);
        const QColor & getColor(void) { return myColor; }
        const double getRelSize(void) { return relSize; }
        const bool getRound(void) { return isRound; }
        void setColor(const QColor &cl = Qt::black) { myColor = cl;  update();}
        void setRelSize(const double rs = 1) { relSize = rs; update();}
        void SetRound(const bool rnd = false) { isRound = rnd; update();}
    protected:
        virtual void paintEvent(QPaintEvent * event);
        virtual QSize sizeHint(void) { return QSize(10,10); };
};

class MainWindow : public QMainWindow
{
private:
    Q_OBJECT
    int errorCnt;
    double scoef[2];    // HV set linear coefficients
    double rcoef[2];    // HV read linear coefficients
    double I0;          // zero current
    double HVset;
public:
    QPushButton *bnLV;
    QPushButton *bnHV;
    QDoubleSpinBox *bxHVRead;
    QDoubleSpinBox *bxHVWrite;
    QDoubleSpinBox *bxIRead;
    QLabel *bxID;
    QDoubleSpinBox *bx1Wire;
    QDoubleSpinBox *bxCPUTemp;
    QDoubleSpinBox *bxDACTemp;
    QPushButton    *bnOn[15];
    QDoubleSpinBox *bxRead[15];
    QDoubleSpinBox *bxWrite[15];
    ColorLed *led;
    QSpinBox *bxAddress;
    QPushButton *bnClear;
    QPushButton *bnReset;
    QPushButton *bnExit;
    QTimer *timer;

    MainWindow(unsigned char addr, QWidget *parent = 0);

private slots:
    void on_bnLV_clicked(bool checked); // automatic...
    void on_bnLV_toggled(bool checked); // automatic...
    void on_bnHV_clicked(bool checked); // automatic...
    void on_bnHV_toggled(bool checked); // automatic...
    void on_bnClear_clicked(bool checked); // automatic...
    void on_bnReset_clicked(bool checked); // automatic...
    void on_bxAddress_valueChanged(int num);
    void on_bxHVWrite_valueChanged(double num);
    void bxWriteChanged(double num);
    void bnOnClicked(bool checked);
    void bnOnToggled(bool checked);
    void onTimer(void);
};

#endif

