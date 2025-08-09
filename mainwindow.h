#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QRegExp>
#include <QMainWindow>
#include <QList>
#include <QVector>
#include <QLineEdit>

#include "config_recorder/uiconfigrecorder.h"

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

typedef enum
{
    TXT_IN_DEC,
    TXT_IN_HEX,
}txt_data_format_e_t;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selFileBtn_clicked();

    void txt_to_img();
    void img_to_txt();

    void on_txtFormatDecRBtn_toggled(bool checked);

    void on_txtFormatHexRBtn_toggled(bool checked);

    void on_genOutPutPBtn_clicked();

    void on_txt2ImgRBtn_toggled(bool checked);

    void on_img2TxtRBtn_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    UiConfigRecorder m_ui_cfg_recorder;
    qobj_ptr_set_t m_ui_cfg_fin, m_ui_cfg_fout;

    internal_px_data_type m_max_px_val;
    data_proc_params_s_t m_data_proc_params;
    txt_data_format_e_t m_txt_data_format;
    QString m_in_file_fpn;
    QRegExp m_sep;
    int m_max_item_cnt_per_line;
    QList<QVector<internal_px_data_type>> m_raw_px_array;

    QString m_sel_dlg_title, m_file_sel_filter_str;

private:
    bool parse_txt_data(QString *ret_str = nullptr);
    bool gen_bin_file(QImage * disp_img = nullptr, QString *info_str = nullptr,
                      QString *ret_str = nullptr);
};
#endif // MAINWINDOW_H
