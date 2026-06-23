#pragma once

#include <QSettings>
#include <QVariant>

// 封装ini进行统一管理
class IniManager
{
public:
    // 获取文件路径进行构造
    explicit IniManager(const QString &filePath);

    /*
     * setValue/Value 负责“快速登记但不保存”，适合批量写入数据，不用sync
     * set/set 负责“登记并且立刻保存”，适合直接存储数据，使用sync
     */

    // QVariant = Qt 的万能数据容器，能存几乎所有类型数据
    // const修饰过的函数不会修改对象的任何非 mutable 成员变量，可以被 const 对象调用，他的作用是保证函数的逻辑状态
    void setValue(const QString &key, const QVariant &val);
    QVariant value(const QString &key, const QVariant &def = {})const;

    // 对外提供的统一接口取值
    template<typename T>
    T get(const QString &key, const T &def = T())const
    {
        QVariant v = m_settings.value(key);
        return v.isValid() ? v.value<T>() : def;
    }

    // 对外提供的统一接口存值
    template<typename T>
    void set(const QString &key, const T &val)
    {
        m_settings.setValue(key, QVariant::fromValue(val));
        m_settings.sync();
    }

    // 判断是否包含key键，返回bool
    bool contains(const QString &key) const;
    // 强缓冲中的数据（ini数据）强制写入磁盘中防止丢失
    void sync();

private:
    // 被 mutable 修饰的成员变量即使在 const 成员函数中也可以被修改
    mutable QSettings m_settings;
};