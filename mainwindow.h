#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QRegExp>
#include <QMainWindow>
#include <QList>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef quint32 internal_px_data_type;

typedef struct
{
    internal_px_data_type append_px_val;
    int skip_head_row_cnt, skip_tail_row_cnt;
    int skip_head_px_cnt, skip_tail_px_cnt;
    QString seperator;
}data_proc_params_s_t;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selTxtFileBtn_clicked();

private:
    Ui::MainWindow *ui;

    internal_px_data_type m_max_px_val;
    data_proc_params_s_t m_data_proc_params;
    QString m_txt_file_fpn;
    QRegExp m_sep;
    int m_max_item_cnt_per_line;
    QList<QVector<internal_px_data_type>> m_raw_px_array;

private:
    bool parse_txt_data(QString *ret_str = nullptr);
    bool gen_bin_file(QString *ret_str = nullptr);
};
#endif // MAINWINDOW_H
