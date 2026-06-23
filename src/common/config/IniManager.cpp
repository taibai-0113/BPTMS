#include "IniManager.h"

IniManager::IniManager(const QString &filePath) : m_settings(filePath, QSettings::IniFormat){}

void IniManager::setValue(const QString &key, const QVariant &val)
{
    m_settings.setValue(key, val);
}

QVariant IniManager::value(const QString &key, const QVariant &def)const
{
    return m_settings.value(key, def);
}

bool IniManager::contains(const QString &key)const
{
    return m_settings.contains(key);
}

void IniManager::sync(){m_settings.sync();}