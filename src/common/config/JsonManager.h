#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

class JsonManager
{
public:
    // 获取json文件路径
    explicit JsonManager(const QString &filePath);

    // 根据路径加载json与保存json
    bool load();
    bool save() const;

    QJsonObject root() const;
    void setRoot(const QJsonObject &obj);
    bool isEmpty()const;

    QJsonValue value(const QString &key)const;
    void setValue(const QString &key, const QJsonValue &val);

private:
    // json文件路径、json对象
    QString m_filePath;
    QJsonObject m_root;
};