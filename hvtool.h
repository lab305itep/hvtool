#ifndef HVTOOL_H
#define HVTOOL_H

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
public:
    QWidget *topframe;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *vlColoumn[3];

    QHBoxLayout *hl[24];
    QLabel *lb[24];
    QSpacerItem *hsp[24];
    QDoubleSpinBox *bxRead[24];
    QPushButton *bnOn[16];
    QDoubleSpinBox *bxWrite[16];
    QCommandLinkButton *bnSet[16];

    QHBoxLayout *hlBottom;
    QLabel *lbLV;
    QPushButton *bnLV;
    QSpacerItem *hspBottom;
    QLabel *lb1WTemp;
    QDoubleSpinBox *bx1WTemp;
    QLabel *lbLed;
    ColorLed *led;
    QLabel *lbAddress;
    QSpinBox *bxAddress;
    QPushButton *bnExit;
    
    QTimer *timer;

    MainWindow(unsigned char addr, QWidget *parent = 0);
    
private slots:
    void on_bnLV_clicked(bool checked); // automatic...
    void on_bnLV_toggled(bool checked); // automatic...
    void on_bxAddress_valueChanged(int num);
    void bnOn_clicked(bool checked);
    void bnOn_toggled(bool checked);
    void bnSet_clicked(void);
    void onTimer(void);
};

#endif

