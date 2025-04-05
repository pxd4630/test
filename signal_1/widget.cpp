#include "widget.h"
#include "ui_widget.h"
#include "QPushButton"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QPushButton* mybutton=new QPushButton(this);
    mybutton->setText("关闭");
    mybutton->move(300,300);
    connect(mybutton,&QPushButton::clicked,this,&QWidget::close);
}

Widget::~Widget()
{
    delete ui;
}
