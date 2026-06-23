#include "JsonManager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>

JsonManager::JsonManager(const QString &filePath) : m_filePath(filePath)
{
    // 构造即加载json文件路径
    load();
}

bool JsonManager::load()
{
    // 打开json路径对应文件并将文件中的数据转为QJsonObject
    QFile f(m_filePath);
    if(!f.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if(doc.isNull() || !doc.isObject()) return false;
    m_root = doc.object();
    return true;
}

bool JsonManager::save()const
{
    // 打开json文件并保存
    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QFile f(m_filePath);
    if(!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(m_root).toJson(QJsonDocument::Indented));
    return true;
}

// Json外部接口
QJsonObject JsonManager::root() const {return m_root;}
void JsonManager::setRoot(const QJsonObject &o) {m_root = o;}
bool JsonManager::isEmpty() const {return m_root.isEmpty();}
QJsonValue JsonManager::value(const QString &k) const {return m_root.value(k);}
void JsonManager::setValue(const QString &k, const QJsonValue &v){m_root.insert(k, v);}