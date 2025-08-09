#include <QtMath>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QDir>
#include <QImage>
#include <QFileDialog>
#include <QPixmap>
#include <QButtonGroup>
#include <QTextStream>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "literal_strings/literal_strings.h"
#include "logger/logger.h"
#include "sysconfigs/sysconfigs.h"

static const char* gs_txt_file_filter_str = "Txt (*.txt)";
static const char* gs_img_file_filter_str = "Images (*.png *.xpm *.jpg *.bmp)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->hisPthInvLEdit->setVisible(false);

    QButtonGroup *txt_format_grp = new QButtonGroup(this);
    txt_format_grp->addButton(ui->txtFormatDecRBtn);
    txt_format_grp->addButton(ui->txtFormatHexRBtn);
    ui->txtFormatDecRBtn->setChecked(true);

    QButtonGroup *trans_type_grp = new QButtonGroup(this);
    trans_type_grp->addButton(ui->txt2ImgRBtn);
    trans_type_grp->addButton(ui->img2TxtRBtn);
    ui->txt2ImgRBtn->setChecked(true);

    m_ui_cfg_fin.clear(); m_ui_cfg_fout.clear();

    m_max_px_val = qPow(2, g_sys_configs_block.bit_per_px) - 1;
    m_sep = QRegExp("[\\s,;:]+");

    m_ui_cfg_recorder.load_configs_to_ui(this, m_ui_cfg_fin, m_ui_cfg_fout);

    m_txt_data_format = ui->txtFormatDecRBtn->isChecked() ? TXT_IN_DEC : TXT_IN_HEX;
    m_file_sel_filter_str = ui->txt2ImgRBtn->isChecked() ? gs_txt_file_filter_str : gs_img_file_filter_str;

    if(ui->txt2ImgRBtn->isChecked()) on_txt2ImgRBtn_toggled(true);
    else on_img2TxtRBtn_toggled(true);
}

MainWindow::~MainWindow()
{
    m_ui_cfg_recorder.record_ui_configs(this, m_ui_cfg_fin, m_ui_cfg_fout);
    delete ui;
}

bool MainWindow::parse_txt_data(QString *ret_str)
{
    QFile txt_file(m_in_file_fpn);
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
    int base = ((TXT_IN_DEC == m_txt_data_format) ? 10 : 16);
    QTextStream txt_stream(&txt_file);
    int row_idx = 0;
    m_raw_px_array.clear();
    while(!txt_stream.atEnd())
    {
        QString line = txt_stream.readLine();
        if(TXT_IN_HEX == m_txt_data_format)
        {
            line = line.toUpper();
            line.replace("0X", "");
        }
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
            px_line[col] = (internal_px_data_type)line_txt_list[col].toUInt(&ok, base);
            if(!ok)
            {
                log_str += QString("第 %1 行第 %2 个数据 %3 非法！\n")
                                    .arg(row_idx).arg(col).arg(line_txt_list[col]);
                all_data_valid = false;
                break;
            }
            else if(px_line[col] > m_max_px_val)
            {
                log_str += QString("第 %1 行第 %2 个数据 %3 超过允许的最大值 %4！\n")
                        .arg(row_idx).arg(col).arg(line_txt_list[col]).arg(m_max_px_val);
                all_data_valid = false;
                break;
            }
        }
        if(!all_data_valid) break;

        m_raw_px_array.append(px_line);
        ++row_idx;
    }
    if(!all_data_valid)
    {
        DIY_LOG(LOG_ERROR, log_str);
        if(ret_str) *ret_str += log_str;
        return false;
    }
    if(m_raw_px_array.size() <= 0)
    {
        log_str += "\n不存在有效数据";
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

bool MainWindow::gen_bin_file(QImage * disp_img, QString *info_str, QString *ret_str)
{
    QString log_str;
    QFileInfo file_info(m_in_file_fpn);
    QString pth = file_info.path();
    QString base_name = file_info.baseName();
    QImage::Format img_type = g_sys_configs_block.bit_per_px > 8 ?
                    QImage::Format_Grayscale16 : QImage::Format_Grayscale8;
    int width = m_max_item_cnt_per_line, height = m_raw_px_array.size();
    QImage img(width, height, img_type);
    QString out_file_names;

    if(0 == width || 0 == height || 0 == m_max_item_cnt_per_line)
    {
        log_str = QString("数据无效：width: %1, height: %2, max_line_len: %3")
                .arg(width).arg(height).arg(m_max_item_cnt_per_line);
        DIY_LOG(LOG_ERROR, log_str);
        if(ret_str) *ret_str = log_str;
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
    out_file_names += base_name + ".png";

    QFile raw_data_file(pth + "/" + base_name + ".raw");
    if(raw_data_file.open(QIODevice::WriteOnly))
    {
        raw_data_file.write(reinterpret_cast<const char *>(img.constBits()), img.sizeInBytes());
        raw_data_file.close();
        out_file_names += QString(" ") + base_name + ".raw";
    }
    else
    {
        log_str += QString("打开文件 %1 失败:%2").arg(raw_data_file.fileName(), raw_data_file.errorString());
        DIY_LOG(LOG_ERROR, log_str);
    }

    QImage img_8bit;
    if(QImage::Format_Grayscale16 == img_type)
    {
        img_8bit = convertGrayscale16To8(img);
        img_8bit.save(pth + "/" + base_name + "-8bit.png");

        out_file_names += QString(" ") + base_name + "-8bit.png";
    }
    else
    {
        img_8bit = img;
    }

    if(ret_str) *ret_str += log_str;

    if(info_str)
    {
        *info_str = QString("图像文件width: %1, height: %2, bit: %3\n保存在 %4%5").arg(width).arg(height)
                .arg((QImage::Format_Grayscale8 == img_type) ? 8 : 16).arg(pth).arg(out_file_names);
    }
    if(disp_img) *disp_img = img_8bit;
    return true;
}

void MainWindow::on_selFileBtn_clicked()
{
    QString dir = ui->hisPthInvLEdit->text().isEmpty() ? "." : ui->hisPthInvLEdit->text();
    m_in_file_fpn = QFileDialog::getOpenFileName(this, m_sel_dlg_title, dir, m_file_sel_filter_str);
    if(m_in_file_fpn.isEmpty())
    {
        return;
    }
    ui->selFileFpnLEdit->setText(m_in_file_fpn);
}


void MainWindow::txt_to_img()
{
    if(m_in_file_fpn.isEmpty())
    {
        QMessageBox::critical(this, "Error", "请先选择文件");
        return;
    }
    QString pth = QFileInfo(m_in_file_fpn).path();
    ui->hisPthInvLEdit->setText(pth);
    QString log_str, info_str;
    ui->imgInfoLbl->setText("正在处理...");
    bool ret = parse_txt_data(&log_str);
    if(!ret)
    {
        ui->imgInfoLbl->clear();
        log_str = QString("错误：") + log_str;
        DIY_LOG(LOG_ERROR, log_str);
        QMessageBox::critical(this, "Error", log_str);
        return;
    }
    log_str.clear();

    QImage disp_img;
    ret = gen_bin_file(&disp_img, &log_str, &info_str);
    if(ret)
    {
        ui->imgInfoLbl->setText(info_str);
        QPixmap scaled = QPixmap::fromImage(disp_img)
                    .scaled(ui->imgDispLbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->imgDispLbl->setPixmap(scaled);
        if(log_str.isEmpty()) log_str = g_str_gen_file_ok;
        else log_str = QString(g_str_gen_file_ok) + "\n" + log_str;
        QMessageBox::information(this, "OK", log_str);
    }
    else
    {
        ui->imgInfoLbl->clear();
        QMessageBox::critical(this, "Error", log_str);
    }
}

void MainWindow::img_to_txt()
{
    QImage img(m_in_file_fpn);
    QString info_str;
    info_str = QString("width: %1, height: %2, depth: %3 bit.").arg(img.width()).arg(img.height())
                                                                    .arg(img.depth());
    if(!img.isGrayscale() && img.format() != QImage::Format_Mono && img.format() != QImage::Format_MonoLSB)
    {
        info_str += "\n非灰度图像！将转为灰度图后处理";
    }

    bool mono = (img.format() == QImage::Format_Mono) || (img.format() == QImage::Format_MonoLSB);
    int min_txt_width = mono ? 1 : 2;
    QFileInfo file_info(m_in_file_fpn);
    QString pth = file_info.path();
    ui->hisPthInvLEdit->setText(pth);
    QString txt_file_fpn = pth + "/" + file_info.baseName();
    txt_file_fpn += QString((TXT_IN_HEX == m_txt_data_format) ? "-hex" : "-dec") + ".txt";

    QFile txt_file(txt_file_fpn);
    if(!txt_file.open(QIODevice::WriteOnly))
    {
        ui->imgInfoLbl->clear();
        QMessageBox::critical(this, "Error", QString("打开文件 %1 失败：%2")
                              .arg(txt_file.fileName(), txt_file.errorString()));
        return;
    }

    ui->imgInfoLbl->setText("正在处理...");

    QTextStream txt_stream(&txt_file);
    QString line_str;
    int base = ((TXT_IN_DEC == m_txt_data_format) ? 10 : 16);
    QImage gray_16bit_img = img.convertToFormat(QImage::Format_Grayscale16);
    for(int row = 0; row < gray_16bit_img.height(); ++row)
    {
        line_str.clear();
        for(int col = 0; col < gray_16bit_img.width(); ++col)
        {
            int px_v = qGray(img.pixel(col, row));
            px_v = mono ? (px_v ? 1 : 0) : px_v;
            if(TXT_IN_HEX == m_txt_data_format)
            {
                line_str += QString::number(px_v, base).rightJustified(min_txt_width, '0') + " ";
            }
            else
            {
                line_str += QString::number(px_v, base) + " ";
            }
        }
        txt_stream << line_str << "\n";
    }

    txt_file.close();

    info_str += QString("\nTxt文件生成成功，保存在 %1").arg(txt_file_fpn);
    ui->imgInfoLbl->setText(info_str);

    QMessageBox::information(this, "OK", g_str_gen_file_ok);
}


void MainWindow::on_txtFormatDecRBtn_toggled(bool checked)
{
    if(checked) m_txt_data_format = TXT_IN_DEC;
}

void MainWindow::on_txtFormatHexRBtn_toggled(bool checked)
{
    if(checked) m_txt_data_format = TXT_IN_HEX;
}


void MainWindow::on_genOutPutPBtn_clicked()
{
    m_in_file_fpn = ui->selFileFpnLEdit->text();
    if(ui->txt2ImgRBtn->isChecked())
    {
        txt_to_img();

    }
    else
    {
        img_to_txt();
    }
}


void MainWindow::on_txt2ImgRBtn_toggled(bool checked)
{
    if(checked)
    {
        QString txt_sel_str = "选择文本文件";
        ui->selFileBtn->setText(txt_sel_str);
        ui->genOutPutPBtn->setText(ui->txt2ImgRBtn->text());
        m_file_sel_filter_str = gs_txt_file_filter_str;
        m_sel_dlg_title = txt_sel_str;

        ui->selFileFpnLEdit->clear();
    }
}


void MainWindow::on_img2TxtRBtn_toggled(bool checked)
{
    if(checked)
    {
        QString txt_sel_str = "选择图像文件";
        ui->selFileBtn->setText(txt_sel_str);
        ui->genOutPutPBtn->setText(ui->img2TxtRBtn->text());
        m_file_sel_filter_str = gs_img_file_filter_str;
        m_sel_dlg_title = txt_sel_str;

        ui->selFileFpnLEdit->clear();
    }
}

