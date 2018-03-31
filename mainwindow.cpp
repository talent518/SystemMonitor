#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <QPainter>
#include <QTimer>
#include <QDateTime>

using namespace std;

void getcpu(cpu_t *cpu) {
    static char buff[128];
    static char strcpu[8];
    FILE *fp;

    memset(cpu, 0, sizeof(cpu_t));

    fp = fopen("/proc/stat", "r");

    memset(buff, 0, sizeof(buff));
    fgets(buff, sizeof(buff) - 1, fp);

    fclose(fp);

    sscanf(buff, "%s%ld%ld%ld%ld%ld%ld%ld%ld%ld", strcpu, &cpu->user, &cpu->nice, &cpu->system, &cpu->idle, &cpu->iowait, &cpu->irq, &cpu->softirq, &cpu->stolen, &cpu->guest);
}

char *names[] = {
    "user",
    "nice",
    "system",
    "idle",
    "iowait",
    "irq",
    "softirq",
    "stolen",
    "guest"
};
QColor colors[] = {
    QColor::fromRgb(0xFF8C00), // user
    QColor::fromRgb(0xFF82AB), // nice
    QColor::fromRgb(0xFF3030), // system
    QColor::fromRgb(0x008B00), // idle
    QColor::fromRgb(0xEE00EE), // iowait
    QColor::fromRgb(0x436EEE), // irq
    QColor::fromRgb(0x7CFC00), // softirq
    QColor::fromRgb(0x63B8FF), // stolen
    QColor::fromRgb(0x333333) // guest
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800, 600);
    pix = QPixmap(width(), height());
    pix.fill(Qt::white);

    pixInfo = QPixmap(150, 235);
    QPainter painter(&pixInfo);
    QFont font(painter.font());
    font.setPixelSize(18);
    font.setBold(false);
    painter.setFont(font);
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(Qt::gray);
    painter.drawRect(QRect(-1, -1, pixInfo.width(), pixInfo.height()));
    for(int i=1;i<=9;i++) {
        painter.setPen(QPen(QBrush(colors[i-1]), 2));
        painter.drawLine(75, 25*i, 140, 25*i);
        painter.setPen(Qt::black);
        painter.drawText(10, 25*i - 5, QString::fromUtf8(names[i-1]));
    }

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    m_pTimer->setTimerType(Qt::PreciseTimer);
    m_pTimer->start(1000);

    cpus.clear();

    getcpu(&cpu);
    all = cpu.user + cpu.nice + cpu.system + cpu.idle + cpu.iowait + cpu.irq + cpu.softirq + cpu.stolen + cpu.guest;

    memset(&percent, 0, sizeof(cpu_percent_t));
}

MainWindow::~MainWindow()
{
    delete ui;
    m_pTimer->stop();
    delete m_pTimer;
}

void MainWindow::handleTimeout()
{
    pix.fill(Qt::white);

    getcpu(&cpu2);

    all2 = cpu2.user + cpu2.nice + cpu2.system + cpu2.idle + cpu2.iowait + cpu2.irq + cpu2.softirq + cpu2.stolen + cpu2.guest;
    double total = (all2 - all) / 100.0;

    percent.user =  (double)(cpu2.user - cpu.user) / total;
    percent.nice =  (double)(cpu2.nice - cpu.nice) / total;
    percent.system = (double)(cpu2.system - cpu.system) / total;
    percent.idle =  (double)(cpu2.idle - cpu.idle) / total;
    percent.iowait = (double)(cpu2.iowait - cpu.iowait) / total;
    percent.irq = (double)(cpu2.irq - cpu.irq) / total;
    percent.softirq = (double)(cpu2.softirq - cpu.softirq) / total;
    percent.stolen = (double)(cpu2.stolen - cpu.stolen) / total;
    percent.guest = (double)(cpu2.guest - cpu.guest) / total;
    cpus.append(percent);

    const unsigned int MAX_COUNT = ceil(width()/5.0);
    if(cpus.count()>MAX_COUNT) {
        cpus.removeFirst();
    }

    QPainter painter(&pix);

    cout << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]").toUtf8().data() << " handleTimeout()" << endl;
    int i, k;
    for(i=0;i<9;i++) {
        QPainterPath path;
        k = MAX_COUNT - cpus.count() +1;
        printf("names(%s):", names[i]);
        bool isFirst = true;
        QPointF sp;
        foreach(cpu_percent_t p, cpus) {
            double d = ((double*)(&p))[i];
            printf(" %5.2f", d);
            QPointF ep(k*5, height() - d/100.0 * (double) height());
            if(ep.y()>=height()) ep.setY(height()-1);
            if(isFirst) {
                isFirst = false;
                path.moveTo(ep);
                sp = ep;
                k++;
                continue;
            }
            QPointF c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
            QPointF c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
            path.cubicTo(c1, c2, ep);
            sp = ep;
            k++;
        }
        printf("\n");

        // 设置画笔颜色
        painter.setPen(QPen(QBrush(colors[i]), 2));
        painter.drawPath(path);
    }

    update();

    cout.flush();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, pix);
    painter.drawPixmap(0, 0, pixInfo);
    painter.setPen(Qt::black);

    QRectF rect(75,0,65,25);
    QFont font(painter.font());
    QTextOption option(Qt::AlignRight|Qt::AlignVCenter);
    int i;
    font.setPixelSize(14);
    painter.setFont(font);
    for(i=0;i<9;i++) {
        double d = ((double*)(&percent))[i];
        rect.setTop(i*25);
        rect.setBottom(rect.top()+25);
        painter.setPen(colors[i]);
        painter.drawText(rect, QString::number(d, 'f', 2), option);
    }
}
