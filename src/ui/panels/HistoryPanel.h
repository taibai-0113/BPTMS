#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include "core/data/BatteryData.h"

class HistoryPanel : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget *parent = nullptr);

    void displayData(const QList<BatteryData> &list);

signals:
    void queryRequested(int taskId);

private slots:
    void onQueryClicked();
    void onExportClicked();

public slots:
    void refreshTaskList();

private:
    void setupUI();

    QComboBox *m_cbTask {nullptr};
    QPushButton *m_btnQuery {nullptr};
    QPushButton *m_btnExport {nullptr};
    QPushButton *m_btnRefresh {nullptr};
    QTableWidget *m_table {nullptr};
    QLabel *m_lblCount {nullptr};

    int currentId = -1;
};