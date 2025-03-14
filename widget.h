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

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_postButton_clicked();

    void on_streamButton_clicked();

private:
    Ui::Widget *ui;

    // 网络管理器
    QNetworkAccessManager *manager;
    // 请求messages
    QJsonArray messageArray;
    QString m_wholeMessage;
    QString m_record;

    void handelReply();
    void sendChatRequest(const QString &api_key, const QString &model, const QString user_message);
    QByteArray buildRequestBody(const QString &message, const QString &model, bool isStream = false);
    QNetworkRequest buildRequestHeader(const QString &api_key);
    QJsonObject parseJsonReplyToMsg(const QByteArray &data, bool isStream = false);
};
#endif // WIDGET_H
