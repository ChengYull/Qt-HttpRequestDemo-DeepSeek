#include "widget.h"
#include "ui_widget.h"
#include <QSslSocket>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    manager = new QNetworkAccessManager(this);

}

Widget::~Widget()
{
    delete ui;
    manager->deleteLater();
}

void Widget::on_pushButton_clicked()
{

    // 创建请求对象 并设置URL
    QNetworkRequest request;
    // 对请求对象 并设置URL
    QUrl url("https://cn.apihz.cn//api/tianqi//tqyb.php?id=88888888&key=88888888&sheng=四川&place=绵阳");
    request.setUrl(url);

    QNetworkReply *reply;

    // 发送Get请求
    reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &Widget::handelReply);
}

void Widget::handelReply(){
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply->error() == QNetworkReply::NoError){
        QByteArray data = reply->readAll();
        qDebug() << "Response: " << data;
    }else{
        qDebug() << "Error: " << reply->errorString();
    }
}

void Widget::on_pushButton_2_clicked()
{
    qDebug() << "support version:" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << manager->supportedSchemes();
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "SSL is not supported!";
        return;
    }

    qDebug() << "SSL library version:" << QSslSocket::sslLibraryVersionString();
}


void Widget::sendChatRequest(const QString &api_key, const QString &model, const QString user_message)
{
    // 构造请求体
    QJsonObject messageObj;
    messageObj.insert("role", "user");
    messageObj.insert("content", user_message);

    messageArray.append(messageObj);

    QJsonObject requestBody;
    requestBody.insert("model", model);
    requestBody.insert("messages", messageArray);

    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();
    qDebug() << "Request JSON:" << doc.toJson(QJsonDocument::Indented);
    // 创建请求
    QNetworkRequest request(QUrl("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"));
    // 设置请求头
    request.setRawHeader("Authorization", ("Bearer " + api_key).toLocal8Bit());
    request.setRawHeader("Content-Type", "application/json");

    // POST请求响应
    QNetworkReply *reply = manager->post(request, data);

    // 连接响应信号
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error()) {
            qDebug() << "Error:" << reply->errorString();
        } else {
            QByteArray response_data = reply->readAll();
            if(response_data.isEmpty()){
                qDebug() << "回复为空";
                return;
            }
            QJsonDocument response_doc = QJsonDocument::fromJson(response_data);
            qDebug() << "Response:" << response_doc.toJson(QJsonDocument::Indented);

            // 解析获取到的Json
            // 获取完整json对象
            QJsonObject rsp_json = response_doc.object();
            // msg对象
            QJsonObject msg = rsp_json.value("choices").toArray()[0].toObject().value("message").toObject();

            QString reasoning = msg.value("reasoning_content").toString();
            ui->reasoning->setText(reasoning);
            QString content = msg.value("content").toString();
            ui->record->append("AI:" + content);
            QJsonObject mes_h;
            mes_h.insert("role", "assistant");
            mes_h.insert("content", content);
            // 将AI回复的内容加入请求头的messages中，AI才会根据上下文回答
            messageArray.append(mes_h);
        }
        reply->deleteLater();
    });
}




void Widget::on_postButton_clicked()
{
    QString mes = ui->userInput->toPlainText();
    if(mes.isEmpty()) return;
    ui->userInput->clear();
    ui->record->append("user: " + mes);
    QString api_key = "sk-a90528958d5d4abb8621ef0123f85f7f";
    QString model = "deepseek-v3";
    sendChatRequest(api_key, model, mes);
}

