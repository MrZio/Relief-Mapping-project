#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myglwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 1. Costruiamo l'interfaccia che hai disegnato su Qt Designer
    ui->setupUi(this);

    // 2. Colleghiamo lo slider alla funzione del cubo (Signal & Slot)
    // - ui->depthSlider è il Mittenete (chi genera l'evento)
    // - &QSlider::valueChanged è il Segnale (il valore è cambiato)
    // - ui->openGLWidget è il Ricevente (il tuo widget OpenGL)
    // - &MyGLWidget::setDepthScale è lo Slot (la funzione che fa il calcolo)

    connect(ui->depthSlider, &QSlider::valueChanged,
            ui->openGLWidget, &MyGLWidget::setDepthScale);
    connect(ui->sizeSlider, &QSlider::valueChanged,
            ui->openGLWidget, &MyGLWidget::setMeshScale);

    // (Opzionale) Se vuoi che la finestra parta di una certa dimensione:
    // resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}