#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include<opencv2\opencv.hpp>
#include<opencv2\dnn.hpp>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QImage *image = NULL;
    QImage *myImage;
    bool flag = 0;
    //QImage myimage(QImage *image);
   void paintEvent(QPaintEvent *);
   void detector();
   cv::Mat QImage2Mat(const QImage& qimage);

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H

