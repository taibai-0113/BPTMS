#pragma once

#include <QString>
#include <QDateTime>

// 五种状态 创建、运行、暂停、完成、中止
enum class TestStatus{Created, Running, Paused, Finished, Aborted};

struct TestRecord
{
    int id{ -1 }; // 设别ID
    QString name{ "" }; // 测试名称
    QString description{ "" }; // 测试描述
    TestStatus status{ TestStatus::Created }; // 测试状态
    QDateTime startTime{}; // 测试开始时间
    QDateTime endTime{}; // 测试结束时间
    int sampleCount{ 0 }; // 采样数量

    // static可以转为inline不会产生重复定义问题
    // 通过测试状态返回字符串
    static QString statusToString(TestStatus  s)
    {
        switch(s)
        {
        case TestStatus::Created: return QStringLiteral("created");
        case TestStatus::Running: return QStringLiteral("running");
        case TestStatus::Paused: return QStringLiteral("paused");
        case TestStatus::Finished: return QStringLiteral("finished");
        case TestStatus::Aborted: return QStringLiteral("aborted");
        default: return QStringLiteral("unknown");
        }
    }

    // 通过字符串返回测试状态
    static TestStatus statusFromString(const QString &s)
    {
        if( s == "created") return TestStatus::Created;
        if( s == "running") return TestStatus::Running;
        if( s == "paused") return TestStatus::Paused;
        if( s == "finished") return TestStatus::Finished;
        if( s == "aborted") return TestStatus::Aborted;
        return TestStatus::Created;
    }

    // 提供一个显式的重置方法
    void reset() {
        *this = TestRecord(); // 利用默认构造函数生成的临时对象进行赋值
    }
};