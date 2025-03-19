#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void requestAI(QJsonArray);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_postButton_clicked();

    void on_streamButton_clicked();

    void on_functionButton_clicked();

    void handel_request(QJsonArray message);
private:
    Ui::Widget *ui;
    // 网络管理器
    QNetworkAccessManager *manager;
    // 请求messages
    QJsonArray m_messageArray;
    QJsonArray m_tools;
    QString m_wholeMessage;
    QString m_record;

    void handelReply();
    void sendChatRequest(const QString &api_key, const QString &model, const QString user_message);
    QByteArray buildRequestBody(const QString &message, const QString &model, bool isStream = false, bool isFunctionCall = false);
    QByteArray buildRequestBody(const QJsonArray &message, const QString &model, bool isStream = false, bool isFunctionCall = false);
    QNetworkRequest buildRequestHeader(const QString &api_key);
    QJsonObject parseJsonReplyToMsg(const QByteArray &data, bool isStream = false);
    QJsonObject executeFunction(const QString &name, const QJsonObject &arguments);
};
#endif // WIDGET_H
