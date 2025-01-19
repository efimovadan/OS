#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRequestFinished(QNetworkReply *reply);
    void onShowAveragesClicked();



private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    void makeRequests();
    void displayTemperatures(const QJsonArray &temperatures);
    void plotGraph(const QJsonArray &temperatures);
    double hourlyAvg = 0.0;
    double dailyAvg = 0.0;
    void makeAvgRequests();
};

#endif // MAINWINDOW_H
