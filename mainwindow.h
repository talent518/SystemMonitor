#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

typedef struct {
    unsigned long int user;
    unsigned long int nice;
    unsigned long int system;
    unsigned long int idle;
    unsigned long int iowait;
    unsigned long int irq;
    unsigned long int softirq;
    unsigned long int stolen;
    unsigned long int guest;
} cpu_t;

typedef struct {
    double user;
    double nice;
    double system;
    double idle;
    double iowait;
    double irq;
    double softirq;
    double stolen;
    double guest;
} cpu_percent_t;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void handleTimeout();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    Ui::MainWindow *ui;
    QPixmap pix, pixInfo;
    QTimer *m_pTimer;
    cpu_t cpu, cpu2;
    unsigned int all, all2;
    QVector<cpu_percent_t> cpus;
    cpu_percent_t percent;
};

#endif // MAINWINDOW_H
