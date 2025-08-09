#include <QtMath>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStringList>
#include <QDir>
#include <QImage>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "logger/logger.h"
#include "sysconfigs/sysconfigs.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_max_px_val = qPow(2, g_sys_configs_block.bit_per_px) - 1;
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::parse_txt_data(QString *ret_str)
{
    QFile txt_file(m_txt_file_fpn);
    QString log_str;

    if(!txt_file.open(QIODevice::ReadOnly))
    {
        log_str += QString("打开文件失败：%1").arg(txt_file.errorString());
        DIY_LOG(LOG_ERROR, log_str);
        if(ret_str) *ret_str += log_str;
        return false;
    }
    bool all_data_valid = true;
    m_max_item_cnt_per_line = 0;
    QTextStream txt_stream(&txt_file);
    int row_idx = 0;
    while(txt_stream.atEnd())
    {
        QString line = txt_stream.readLine();
        QStringList line_txt_list = line.split(m_sep, Qt::SkipEmptyParts);
        int item_cnt = line_txt_list.size();
        if(item_cnt > m_max_item_cnt_per_line)
        {
            m_max_item_cnt_per_line = item_cnt;
        }
        QVector<internal_px_data_type> px_line(item_cnt);
        bool ok;
        for(int col = 0; col < item_cnt; ++col)
        {
            px_line[col] = (internal_px_data_type)line_txt_list[col].toUInt(&ok);
            if(!ok)
            {
                log_str += QString("第 %1 行第 %2 个数据 %3 非法！\n")
                                    .arg(row_idx).arg(col).arg(line_txt_list[col]);
                all_data_valid = false;
            }
            else if(px_line[col] > m_max_px_val)
            {
                log_str += QString("第 %1 行第 %2 个数据 %3 超过允许的最大值 %4！\n")
                        .arg(row_idx).arg(col).arg(line_txt_list[col]).arg(m_max_px_val);
                all_data_valid = false;
            }
        }
        m_raw_px_array.append(px_line);
        ++row_idx;
    }
    if(!all_data_valid)
    {
        DIY_LOG(LOG_ERROR, log_str);
        if(ret_str) *ret_str += log_str;
        return false;
    }
    for(int row = 0; row < m_raw_px_array.size(); ++row)
    {
        QVector<internal_px_data_type> & v_ref = m_raw_px_array[row];
        if(v_ref.size() < m_max_item_cnt_per_line)
        {
            int ori_size = v_ref.size();
            v_ref.resize(m_max_item_cnt_per_line);

            std::fill(v_ref.begin() + ori_size, v_ref.end(), m_data_proc_params.append_px_val);
        }
    }
    log_str += "解析文本文件成功";
    if(ret_str) *ret_str += log_str;
    return true;
}

bool MainWindow::gen_bin_file(QString * ret_str)
{
    QString log_str;
    QFileInfo file_info(m_txt_file_fpn);
    QString pth = file_info.path();
    QString base_name = file_info.baseName();
    QImage::Format img_type = g_sys_configs_block.bit_per_px > 8 ?
                    QImage::Format_Grayscale16 : QImage::Format_Grayscale8;
    int width = m_max_item_cnt_per_line, height = m_raw_px_array.size();
    QImage img(width, height, img_type);

    if(0 == width || 0 == height || 0 == m_max_item_cnt_per_line)
    {
        return false;
    }

    for (int y = 0; y < height; ++y)
    {
        const QVector<internal_px_data_type>& row = m_raw_px_array[y];
        uchar* line = img.scanLine(y);
        for (int x = 0; x < width; ++x)
        {
            if(QImage::Format_Grayscale16 == img_type)
            {
                quint16 gray = static_cast<quint16>(row[x]);
                // 写入16位灰度（低字节在前）
                line[2 * x]     = gray & 0xFF;         // LSB
                line[2 * x + 1] = (gray >> 8) & 0xFF;  // MSB
            }
            else
            {
                quint8 gray = static_cast<quint16>(row[x]);
                line[x] = gray;
            }
        }
    }
    img.save(pth + "/" + base_name + ".png");

    QFile raw_data_file(pth + "/" + base_name + ".raw");
    if(raw_data_file.open(QIODevice::WriteOnly))
    {
        raw_data_file.write(reinterpret_cast<const char *>(img.constBits()), img.sizeInBytes());
        raw_data_file.close();
    }
    else
    {
        log_str += QString("write %1 fails: open file error: %2").arg(raw_data_file.fileName(),
                                                  raw_data_file.errorString());
        DIY_LOG(LOG_ERROR, log_str);
    }

    if(QImage::Format_Grayscale16 == img_type)
    {
        QImage img_8bit = convertGrayscale16To8(img);
        img_8bit.save(pth + "/" + base_name + "-8bit.png");
    }
    if(!log_str.isEmpty()) log_str += "\n";
    log_str += "Generate image file ok.";

    if(ret_str) *ret_str += log_str;
    return true;
}

void MainWindow::on_selTxtFileBtn_clicked()
{
    QFileDialog::getOpenFileName(this, "选择文本文件", "./", tr("Txt (*.txt)"));
}

