#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myglwidget.h"
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);   // <-- OBBLIGATORIO: inizializza tutti i widget del .ui
    //     deve essere la PRIMA riga del costruttore

    // Slider 1: Depth scale
    ui->depthSlider->setRange(0, 100);
    ui->depthSlider->setValue(16);
    connect(ui->depthSlider, &QSlider::valueChanged,
            ui->openGLWidget, &MyGLWidget::setDepthScale);

    // Slider 2: Linear steps
    ui->stepsSlider->setRange(0, 100);
    ui->stepsSlider->setValue(20);
    connect(ui->stepsSlider, &QSlider::valueChanged,
            ui->openGLWidget, &MyGLWidget::setLinearSteps);

    // Checkbox: toggle Linear only / Linear + Binary
    ui->searchModeCheck->setChecked(true);
    connect(ui->searchModeCheck, &QCheckBox::toggled,
            this, [this](bool checked) {
                ui->openGLWidget->setSearchMode(checked ? 1 : 0);
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}