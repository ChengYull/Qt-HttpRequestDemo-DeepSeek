#include "widget.h"
#include "ui_widget.h"
#include <QSslSocket>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    manager = new QNetworkAccessManager(this);
    QJsonObject msg_sys;
    msg_sys.insert("role", "system");
    msg_sys.insert("content", "你是高强度贴吧冲浪者，善于使用孙吧等常用话术，回答问题简明扼要，尖酸刻薄");
    messageArray.append(msg_sys);
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

            // msg对象
            QJsonObject msg = parseJsonReplyToMsg(response_data);

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

QNetworkRequest Widget::buildRequestHeader(const QString &api_key)
{
    // 构建请求头
    QNetworkRequest request(QUrl("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"));
    request.setRawHeader("Authorization", ("Bearer " + api_key).toLocal8Bit());
    request.setRawHeader("Content-Type", "application/json");
    return request;
}

QByteArray Widget::buildRequestBody(const QString &message, const QString &model, bool isStream)
{
    // 构造请求体
    QJsonObject messageObj;
    messageObj.insert("role", "user");
    messageObj.insert("content", message);

    messageArray.append(messageObj);

    QJsonObject requestBody;
    requestBody.insert("model", model);
    requestBody.insert("messages", messageArray);

    if(isStream){
        requestBody.insert("stream", true);
        QJsonObject stream_options;
        stream_options.insert("include_usage", true);
        requestBody.insert("stream_options", stream_options);
    }
    QJsonDocument doc(requestBody);
    qDebug() << "Request_stream JSON:" << doc.toJson(QJsonDocument::Indented);
    QByteArray data = doc.toJson();
    return data;
}

QJsonObject Widget::parseJsonReplyToMsg(const QByteArray &data, bool isStream)
{
    QJsonDocument response_doc = QJsonDocument::fromJson(data);
    // 解析获取到的Json 获取完整json对象
    QJsonObject rsp_json = response_doc.object();
    // msg对象
    QJsonObject msg;

    if(!isStream)
        msg = rsp_json.value("choices").toArray()[0].toObject().value("message").toObject();
    else
        msg = rsp_json.value("choices").toArray()[0].toObject().value("delta").toObject();

    return msg;
}




void Widget::on_postButton_clicked()
{
    QString mes = ui->userInput->toPlainText();
    if(mes.isEmpty()) return;
    ui->userInput->clear();
    ui->record->append("user: " + mes);
    QString api_key = "sk-a90528958d5d4abb8621ef0886f85f7f";
    QString model = "deepseek-v3";
    sendChatRequest(api_key, model, mes);
}


void Widget::on_streamButton_clicked()
{
    m_wholeMessage.clear();
    QString api_key = "sk-a90528958d5d4abb8621ef0886f85f7f";
    QString model = "deepseek-v3";
    QString message = ui->userInput->toPlainText();
    ui->userInput->clear();
    ui->record->append("user: " + message);
    if(message.isEmpty()) return;
    // 请求头
    QNetworkRequest requestHeader = buildRequestHeader(api_key);
    // 请求体
    QByteArray requestBody = buildRequestBody(message, model, true);
    // 发送post请求
    QNetworkReply *reply1 = manager->post(requestHeader, requestBody);

    ui->record->append("AI:");
    m_record = ui->record->toPlainText();
    // 连接响应信号
    connect(reply1, &QNetworkReply::readyRead, this, [this, reply1]() {

        // 通过mid(6)去除 'data: ' 前缀 --- 舍弃，可能同时收到2条data
        // QByteArray response_data = reply1->readAll().mid(6);

        QByteArray response_data = reply1->readAll();
        qDebug() << "Response_data:" << QString::fromStdString(response_data.toStdString());
        if(response_data.isEmpty()){
            qDebug() << "回复为空";
            return;
        }
        QByteArray tmp_data = QByteArray(response_data);
        int pos = response_data.indexOf("\n\n");
        while(pos != -1){
            QByteArray chunk = tmp_data.left(pos);
            QString msg = parseJsonReplyToMsg(chunk.mid(6), true).value("content").toString();
            m_wholeMessage.append(msg);
            if(pos + 2 >= tmp_data.size()) break;
            tmp_data = tmp_data.mid(pos + 2);
            pos = tmp_data.indexOf("\n\n");
        }


        /*
        // msg对象
        QJsonObject msg = parseJsonReplyToMsg(response_data, true);
        //qDebug() << "Response_stream_msg:" << msg;
        QString content = msg.value("content").toString();
        // 注意这里m_record 和 m_wholeMessage 不能使用局部变量， 使用局部变量时，在按钮释放后就会被释放掉导致程序崩溃
        m_wholeMessage.append(content);
        */
        ui->record->clear();
        ui->record->setText(m_record + m_wholeMessage);

    }, Qt::QueuedConnection);    // 使用排队连接确保线程安全

    connect(reply1, &QNetworkReply::finished, this, [this, reply1]() {
        if (reply1->error()) {
            qDebug() << "Error:" << reply1->errorString();
            return;
        }
        qDebug() << "whole mes:" << m_wholeMessage;
        reply1->deleteLater();
        QJsonObject mes_h;
        mes_h.insert("role", "assistant");
        mes_h.insert("content", m_wholeMessage);
        // 将AI回复的内容加入请求头的messages中
        messageArray.append(mes_h);
    });
}

