#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <QtCharts/QDateTimeAxis>
#include <QTableWidget>
#include <QDialog>
#include <QHeaderView>
#include <QLabel>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))  // создаем объект для работы с сетью
{
    ui->setupUi(this);
    setWindowTitle("Temperature Viewer");
    makeRequests();
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onRequestFinished);

    connect(ui->showAveragesButton, &QPushButton::clicked, this, &MainWindow::onShowAveragesClicked);

}

void MainWindow::onShowAveragesClicked()
{

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Averages");
    dialog->resize(300, 200);

    QLabel *hourlyLabel = new QLabel(QString("Hourly Average: %1 °C").arg(hourlyAvg), dialog);
    QLabel *dailyLabel = new QLabel(QString("Daily Average: %1 °C").arg(dailyAvg), dialog);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(hourlyLabel);
    layout->addWidget(dailyLabel);
    dialog->setLayout(layout);

    dialog->exec();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeRequests()
{
    QUrl allTemperaturesUrl("http://localhost:8080/all_temperatures");
    networkManager->get(QNetworkRequest(allTemperaturesUrl));

    QUrl hourlyAvgUrl("http://localhost:8080/hourly_avg");
    networkManager->get(QNetworkRequest(hourlyAvgUrl));

    QUrl dailyAvgUrl("http://localhost:8080/daily_avg");
    networkManager->get(QNetworkRequest(dailyAvgUrl));
}


void MainWindow::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Request failed: " << reply->url() << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    QString url = reply->url().toString();

    if (url.contains("all_temperatures")) {
        QJsonArray jsonArray = jsonDoc.array();
        plotGraph(jsonArray);

    } else if (url.contains("hourly_avg")) {
        QJsonObject jsonObj = jsonDoc.object();
        hourlyAvg = jsonObj["average"].toDouble();
        qDebug() << "Hourly Average: " << hourlyAvg;

    } else if (url.contains("daily_avg")) {
        QJsonObject jsonObj = jsonDoc.object();
        dailyAvg = jsonObj["average"].toDouble();
        qDebug() << "Daily Average: " << dailyAvg;

    } else {
        qWarning() << "Unknown request URL: " << url;
    }

    reply->deleteLater();
}


void MainWindow::plotGraph(const QJsonArray &temperatures)
{
    qDebug() << "Size: " << temperatures.size() << "\n";
    QLineSeries *series = new QLineSeries();

    QJsonObject firstTempObject = temperatures.first().toObject();
    QString firstTimestamp = firstTempObject["timestamp"].toString();
    QDateTime firstDateTime = QDateTime::fromString(firstTimestamp, "yyyy-MM-dd HH:mm:ss");

    for (const QJsonValue &value : temperatures) {
        QJsonObject tempObject = value.toObject();

        double temperature = tempObject["temperature"].toDouble();
        QString timestamp = tempObject["timestamp"].toString();

        QDateTime dateTime = QDateTime::fromString(timestamp, "yyyy-MM-dd HH:mm:ss");
        qint64 timeDiff = (dateTime.toMSecsSinceEpoch() - firstDateTime.toMSecsSinceEpoch()) / 1000;


        QDateTime adjustedDateTime = firstDateTime.addSecs(timeDiff);


        series->append(adjustedDateTime.toMSecsSinceEpoch(), temperature);
        //qDebug() << "Temp: " << temperature << " Timestamp: " << adjustedDateTime.toString("HH:mm:ss") << "\n";
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Temperatures Over Time");

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("HH:mm:ss");
    axisX->setTitleText("Time");

    axisX->setRange(firstDateTime, firstDateTime.addSecs((temperatures.last().toObject()["timestamp"].toString()).toInt()));


    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Temperature (°C)");

    double minTemp = series->at(0).y();
    double maxTemp = series->at(0).y();

    for (int i = 1; i < series->count(); ++i) {
        double temp = series->at(i).y();
        minTemp = qMin(minTemp, temp);
        maxTemp = qMax(maxTemp, temp);
    }

    axisY->setRange(minTemp - 5, maxTemp + 5);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout->addWidget(chartView);
}
