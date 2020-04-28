#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QPainter>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include<opencv2\opencv.hpp>
#include<opencv2\dnn.hpp>
#include <iostream>
#include<map>
#include<string>
#include<time.h>
#include<QPixmap>
#include<QFile>
#include<QFileDialog>
#include<QMessageBox>
#include<QTextStream>


using namespace std;
using namespace cv;

const size_t inWidth = 300;
const size_t inHeight = 300;
const float WHRatio = inWidth / (float)inHeight;
const char* classNames[]= {"background", "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
"fire hydrant", "background", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "background", "backpack",
"umbrella", "background", "background", "handbag", "tie", "suitcase", "frisbee","skis", "snowboard", "sports ball", "kite", "baseball bat","baseball glove", "skateboard", "surfboard", "tennis racket",
"bottle", "background", "wine glass", "cup", "fork", "knife", "spoon","bowl", "banana",  "apple", "sandwich", "orange","broccoli", "carrot", "hot dog",  "pizza", "donut",
"cake", "chair", "couch", "potted plant", "bed", "background", "dining table", "background", "background", "toilet", "background","tv", "laptop", "mouse", "remote", "keyboard",
"cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "background","book", "clock", "vase", "scissors","teddy bear", "hair drier", "toothbrush"};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(800,800);
    QImage *image = new QImage;
    ui->label->setAlignment(Qt::AlignCenter);

//实现本地上传按钮功能，加载本地路径
    connect(ui->pushButton_2,&QPushButton::clicked,[=](){

        QString path = QFileDialog::getOpenFileName(this,"打开图片",".", "图片 (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);所有文件(*.*)");
        if (path != "")
        {
            flag = 1;
            if(image->load(path))
              {
                  ui->label->setPixmap(QPixmap::fromImage((*image).scaled(ui->label->size(), Qt::KeepAspectRatio)));
                  flag = 1;
                  ui->textEdit->setText("");
              }
              else
              {
                   QMessageBox::information(this,"打开图像失败","打开图像失败!");
                   return;
              }
        }
         else
            flag = 0;
        });
    this->myImage = image;

//实现检测按钮功能，按下检测按钮实现检测功能
    connect(ui->pushButton,&QPushButton::clicked,[=](){
        if(flag)
        {
            detector();
        }
        else
        {
            QMessageBox::information(this,"选择图片","请先上传图片!");
            return;
        }

    });




}
//SSD检测过程
void MainWindow::detector(){
       clock_t start, finish;
       double totaltime;
       Mat frame;

       String weights = "./ssd_mobilenet_v1_coco_11_06_2017/frozen_inference_graph.pb";
       String prototxt = "./ssd_mobilenet_v1_coco_11_06_2017/ssd_mobilenet_v1_coco.pbtxt";
       dnn::Net net = cv::dnn::readNetFromTensorflow(weights, prototxt);
       frame = QImage2Mat(*myImage);

       start = clock();
       Size frame_size = frame.size();

       Size cropSize;
       if (frame_size.width / (float)frame_size.height > WHRatio)
       {
           cropSize = Size(static_cast<int>(frame_size.height * WHRatio),
                           frame_size.height);
       }
       else
       {
           cropSize = Size(frame_size.width,
                           static_cast<int>(frame_size.width / WHRatio));
       }

       Rect crop(Point((frame_size.width - cropSize.width) / 2,
                       (frame_size.height - cropSize.height) / 2),
                 cropSize);


       cv::Mat blob = cv::dnn::blobFromImage(frame, 1. / 255, Size(300, 300));
       //blobFromImage主要是用来对图片进行预处理。包含两个主要过程：
       //整体像素值减去平均值（mean）
       //通过缩放系数（scalefactor）对图片像素值进行缩放

       // 1*3*300*300
       cout << "blob size: " << blob.size << endl;

       net.setInput(blob);
       Mat output = net.forward();
       // 1*1*100*7
       cout << "output size: " << output.size << endl;

       Mat detectionMat(output.size[2], output.size[3], CV_32F, output.ptr<float>());

       frame = frame(crop);
       float confidenceThreshold = 0.60;
       for (int i = 0; i < detectionMat.rows; i++)
       {
           float confidence = detectionMat.at<float>(i, 2);

           if (confidence > confidenceThreshold)
           {
               size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

               int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
               int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
               int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
               int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

               ostringstream ss;
               ss << confidence;
               String conf(ss.str());

               Rect object((int)xLeftBottom, (int)yLeftBottom,
                           (int)(xRightTop - xLeftBottom),
                           (int)(yRightTop - yLeftBottom));

               rectangle(frame, object, Scalar(0, 255, 0), 1);
               cout << "objectClass:" << objectClass << endl;
               String label = String(classNames[objectClass]) + ": " + conf;
               cout << "label"<<label << endl;
               //重定义标准输出流
               QTextStream stream (stdout);
               QString str ;
               //把string类型转化为QString
               str = QString::fromStdString(label);
               stream << str<<endl;
               ui->textEdit->append(str +"\n");
               //QString转化成string
               //label = str.toStdString();

               int baseLine = 0;
               Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
               rectangle(frame, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
                                     Size(labelSize.width, labelSize.height + baseLine)),
                         Scalar(0, 255, 0), CV_FILLED);
               putText(frame, label, Point(xLeftBottom, yLeftBottom),
                       FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
           }
       }
       finish = clock();
       totaltime = finish - start;
       cout << "Recognition time " << totaltime <<"ms"<< endl;
       // namedWindow("result", 0);
       //imshow("result", frame);

       //将SSD处理后的mat格式转化成QImage形式
           cvtColor(frame, frame, COLOR_BGR2RGB);
           int width=frame.cols;
           int height=frame.rows;
           int i,j;
           QImage *Image = new QImage(width,height,QImage::Format_RGB888);
           for(i=0;i<height;i++)
           {
               unsigned char *ptr = frame.ptr<unsigned char>(i);
               //unsigned char *ptr = img.ptr<unsigned char>(i)[0];
               for(j=0;j<width*3;j+=3)
               {
                   Image->bits()[(Image->bytesPerLine()*i)+(j+2)] =  ptr[j+2];
                   Image->bits()[(Image->bytesPerLine()*i)+(j+1)] =  ptr[j+1];
                   Image->bits()[(Image->bytesPerLine()*i)+(j)] = ptr[j];
               }
           }

      // cvtColor(frame, frame, COLOR_BGR2RGB);//图像格式转换
      // QImage disImage = QImage((const unsigned char*)(frame.data),frame.cols,frame.rows,QImage::Format_RGB888); 不管用
        //ui->label->setAlignment(Qt::AlignHCenter);
        ui->label->setPixmap(QPixmap::fromImage((*Image).scaled(ui->label->size(), Qt::KeepAspectRatio)));//显示图像

}

Mat MainWindow::QImage2Mat(const QImage& qimage)
{
    Mat mat = Mat(qimage.height(), qimage.width(), CV_8UC4, (uchar*)qimage.bits(), qimage.bytesPerLine());
    Mat mat2 = Mat(mat.rows, mat.cols, CV_8UC3 );
    int from_to[] = { 0,0, 1,1, 2,2 };
    mixChannels( &mat, 1, &mat2, 1, from_to, 3 );
    return mat2;
}

//画家功能实现，加载图片
void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix;
    pix.load(":/res/bg.jpg");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);
}


MainWindow::~MainWindow()
{
    delete ui;
}

