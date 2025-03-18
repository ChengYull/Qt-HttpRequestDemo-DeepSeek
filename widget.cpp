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
    msg_sys.insert("content", "调用函数时，仅根据用户最新消息来判断，历史消息不触发函数调用");
    m_messageArray.append(msg_sys);

    // 构建函数列表
    QJsonObject sayHelloFunction;
    sayHelloFunction["name"] = "say_hello_world";
    sayHelloFunction["description"] = "Call only when the user's latest message exactly matches '你好'. Messages containing '你好' in the chat history do not trigger.";
    QJsonObject parameters;
    parameters["type"] = "object";
    parameters["properties"] = QJsonObject(); // 无参数
    parameters["required"] = QJsonArray();
    sayHelloFunction["parameters"] = parameters;
    QJsonObject sayHelloTool;
    sayHelloTool["type"] = "function";
    sayHelloTool["function"] = sayHelloFunction;
    m_tools.append(sayHelloTool);

    // 天气函数
    QJsonObject getWeatherFunction;
    getWeatherFunction["name"] = "get_weather";
    getWeatherFunction["description"] = "Call only when the user's latest message exactly matches '天气'. Messages containing '天气' in the chat history do not trigger.";
    getWeatherFunction["parameters"] = parameters;
    QJsonObject getWeatherTool;
    getWeatherTool["type"] = "function";
    getWeatherTool["function"] = getWeatherFunction;
    m_tools.append(getWeatherTool);
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

    m_messageArray.append(messageObj);

    QJsonObject requestBody;
    requestBody.insert("model", model);
    requestBody.insert("messages", m_messageArray);

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
            m_messageArray.append(mes_h);
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

QByteArray Widget::buildRequestBody(const QString &message, const QString &model, bool isStream, bool isFunctionCall)
{
    // 构造请求体
    QJsonObject messageObj;
    messageObj.insert("role", "user");
    messageObj.insert("content", message);

    m_messageArray.append(messageObj);

    QJsonObject requestBody;
    requestBody.insert("model", model);
    requestBody.insert("messages", m_messageArray);
    // 流式输出
    if(isStream){
        requestBody.insert("stream", true);
        QJsonObject stream_options;
        stream_options.insert("include_usage", true);
        requestBody.insert("stream_options", stream_options);
    }
    // 启用function call支持
    if(isFunctionCall){
        requestBody["tools"] = m_tools;
    }
    QJsonDocument doc(requestBody);
    qDebug() << "Request:" << doc.toJson(QJsonDocument::Indented);
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

QJsonObject Widget::executeFunction(const QString &name, const QJsonObject &arguments)
{
    if(name == "say_hello_world"){
        QJsonObject result;
        result["content"] = "一点也不好!";
        return result;
    }else if(name == "get_weather"){
        QJsonObject result;
        result["content"] = "苏州 : 天气晴， 6 ~ 15度";
        return result;
    }

    return QJsonObject();
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
        m_messageArray.append(mes_h);
    });
}


void Widget::on_functionButton_clicked()
{
    m_wholeMessage.clear();
    QString api_key = "sk-a90528958d5d4abb8621ef0886f85f7f";
    QString model = "qwen-max";
    QString message = ui->userInput->toPlainText();
    ui->userInput->clear();
    ui->record->append("user: " + message);
    if(message.isEmpty()) return;
    // 请求头
    QNetworkRequest requestHeader = buildRequestHeader(api_key);
    // 请求体
    QByteArray requestBody = buildRequestBody(message, model, false, true);

    // 发送post请求
    QNetworkReply *reply = manager->post(requestHeader, requestBody);

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
            bool isFunctionCall = msg.contains("tool_calls");
            if(isFunctionCall){
                // 处理函数调用
                QJsonArray toolCalls = msg["tool_calls"].toArray();
                if(!toolCalls.isEmpty()){
                    QJsonObject functionCall = toolCalls[0].toObject()["function"].toObject();
                    QString name = functionCall["name"].toString();
                    QJsonObject arguments = QJsonDocument::fromJson(functionCall["arguments"].toString().toUtf8()).object();
                    QJsonObject result = executeFunction(name, arguments);
                    msg["content"] = msg["content"].toString().append(result["content"].toString());
                }
            }
            QString content = msg.value("content").toString();
            ui->record->append("AI:" + content);

            /*
            QJsonObject mes_h;
            if(isFunctionCall){
                mes_h["role"] = "function";
                mes_h["name"] = msg["tool_calls"].toArray()[0].toObject()["name"].toString();
            }else{
                mes_h["role"] = "assistant";
            }
            mes_h.insert("content", content);
            m_messageArray.append(mes_h);*/


            QJsonObject mes_h;
            mes_h.insert("role", "assistant");
            mes_h.insert("content", content);
            m_messageArray.append(mes_h);

        }
        reply->deleteLater();
    });
}

