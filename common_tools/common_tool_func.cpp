#include <QDateTime>
#include <QList>
#include <QProcess>
#include <QDir>
#include <QColor>
#include <QFont>
#include <QtMath>
#include <QStorageInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "literal_strings/literal_strings.h"
#include "sysconfigs/sysconfigs.h"
#include "common_tool_func.h"
#include "logger/logger.h"

static bool exec_external_process(QString cmd, QString cmd_args, bool as_admin = false)
{
    DIY_LOG(LOG_INFO, QString("exec_external_process: %1 %2, as_admin: %3")
                      .arg(cmd, cmd_args).arg((int)as_admin));
    bool ret = false;
#ifdef Q_OS_WIN
    if(!cmd.isEmpty())
    {
        SHELLEXECUTEINFO shellInfo;
        memset(&shellInfo, 0, sizeof(shellInfo));
        shellInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        shellInfo.hwnd = NULL;
        std::wstring wlpstrstd_verb;
        if(as_admin)
        {
            wlpstrstd_verb = QString("runas").toStdWString();
        }
        else
        {
            wlpstrstd_verb = QString("open").toStdWString();
        }
        shellInfo.lpVerb = wlpstrstd_verb.c_str();
        std::wstring wlpstrstd_file = cmd.toStdWString();
        shellInfo.lpFile = wlpstrstd_file.c_str();
        std::wstring wlpstrstd_param = cmd_args.toStdWString();
        shellInfo.lpParameters = wlpstrstd_param.c_str();
        std::wstring wlpstrstd_currpth = QDir::currentPath().toStdWString();
        shellInfo.lpDirectory = wlpstrstd_currpth.c_str();
        shellInfo.nShow = SW_HIDE;
        BOOL bRes = ::ShellExecuteEx(&shellInfo);
        if(bRes)
        {
            ret = true;
            DIY_LOG(LOG_INFO, "ShellExecuteEx ok.");
        }
        else
        {
            ret = false;
            DWORD err = GetLastError();
            DIY_LOG(LOG_ERROR, QString("ShellExecuteEx return false, error: %1").arg((quint64)err));
        }
    }
    else
    {
        DIY_LOG(LOG_WARN, QString("ShellExecuteEx, cmd is empty!"));
    }
#elif defined(Q_OS_UNIX)
#else
#endif
    return ret;
}

QString common_tool_get_curr_dt_str()
{
    QDateTime curDateTime = QDateTime::currentDateTime();
    QString dtstr = curDateTime.toString("yyyyMMddhhmmss");
    return dtstr;
}

QString common_tool_get_curr_date_str()
{
    QDateTime curDateTime = QDateTime::currentDateTime();
    QString datestr = curDateTime.toString("yyyyMMdd");
    return datestr;
}

QString common_tool_get_curr_time_str()
{
    QTime curTime = QTime::currentTime();
    QString time_str = curTime.toString("hh:mm:ss");
    return time_str;
}

bool mkpth_if_not_exists(const QString &pth_str)
{
    QDir data_dir(pth_str);
    bool ret = true;
    if(!data_dir.exists())
    {
        data_dir = QDir();
        ret = data_dir.mkpath(pth_str);
    }
    return ret;
}

QString shutdown_system(QString reason_str,int wait_time)
{
#ifdef Q_OS_UNIX
    extern const char* g_main_th_local_log_fn;
    LOCAL_DIY_LOG(LOG_INFO, g_main_th_local_log_fn, reason_str);

    if(wait_time > 0) QThread::sleep(wait_time);

    QProcess::execute("sync", {});
    QProcess::execute("systemctl", {"poweroff"});

    return "";
#else
    QString s_c = "shutdown";
    QStringList ps_a;
    QProcess ps;
    ps.setProgram(s_c);
    ps_a << "/s" << "/t" << QString("%1").arg(wait_time);
    ps_a << "/d" << "u:4:1" << "/c" << reason_str;
    ps.setArguments(ps_a);
    ps.startDetached();
    return s_c + " " + ps_a.join(QChar(' '));
#endif
}

/*begin of RangeChecker------------------------------*/
#undef EDGE_ITEM
#define EDGE_ITEM(a) #a
template <typename T> const char* RangeChecker<T>::range_edge_strs[] =
{
    EDGE_LIST
};
#undef EDGE_ITEM
static const char* gs_range_checker_err_msg_invalid_edge_p1 = "low/up edge should be";
static const char* gs_range_checker_err_msg_invalid_edge_p2 = "and can't both be";
static const char* gs_range_checker_err_msg_invalid_eval =
        "Invalid range: min must be less than or equal to max!";

template <typename T>
RangeChecker<T>::RangeChecker(T min, T max, QString unit_str,
                           range_edge_enum_t low_edge, range_edge_enum_t up_edge)
{
    if((low_edge > EDGE_INFINITE) || (up_edge > EDGE_INFINITE)
        || (low_edge < EDGE_INCLUDED) || (up_edge < EDGE_INCLUDED)
        || (EDGE_INFINITE == low_edge && EDGE_INFINITE == up_edge))
    {
        DIY_LOG(LOG_ERROR, QString("%1 %2, %3, or %4, %5").arg(
                    gs_range_checker_err_msg_invalid_edge_p1,
                    range_edge_strs[EDGE_INCLUDED],
                    range_edge_strs[EDGE_EXCLUDED],
                    range_edge_strs[EDGE_INFINITE],
                    gs_range_checker_err_msg_invalid_edge_p2
                    ));
        return;
    }
    valid = (EDGE_INFINITE == low_edge || EDGE_INFINITE == up_edge) ? true : (min <= max);
    this->min = min; this->max = max;
    this->low_edge = low_edge; this->up_edge = up_edge;
    this->unit_str = unit_str;

    if(!valid)
    {
        DIY_LOG(LOG_ERROR, QString(gs_range_checker_err_msg_invalid_eval));
    }
}

template <typename T> bool RangeChecker<T>::range_check(T val)
{
    bool ret = true;
    if(!valid)
    {
        DIY_LOG(LOG_ERROR, "Invalid Range Checker!")
        return false;
    }

    if(EDGE_INCLUDED == low_edge) ret = (ret && (val >= min));
    else if(EDGE_EXCLUDED == low_edge) ret = (ret && (val > min));

    if(EDGE_INCLUDED == up_edge) ret = (ret && (val <= max));
    else if(EDGE_EXCLUDED == up_edge) ret = (ret && (val < max));
    return ret;
}

template <typename T> range_edge_enum_t RangeChecker<T>::range_low_edge()
{
    return low_edge;
}

template <typename T> range_edge_enum_t RangeChecker<T>::range_up_edge()
{
    return up_edge;
}

template <typename T> T RangeChecker<T>::range_min()
{
    return min;
}

template <typename T> T RangeChecker<T>::range_max()
{
    return max;
}

template <typename T> QString RangeChecker<T>::
range_str(common_data_type_enum_t d_type, double factor, QString new_unit_str )
{
    QString ret_str;

    if(!valid) return ret_str;

    ret_str = (EDGE_INCLUDED == low_edge ? "[" : "(");
    ret_str += (EDGE_INFINITE == low_edge) ? "" :
                ((INT_DATA == d_type) ? QString::number((int)(min * factor)) :
                                        QString::number((float)(min * factor)));
    ret_str += ", ";
    ret_str += (EDGE_INFINITE == up_edge) ? "" :
                ((INT_DATA == d_type) ? QString::number((int)(max * factor)) :
                                        QString::number((float)(max * factor)));
    ret_str += (EDGE_INCLUDED == up_edge) ? "]" : ")";
    //ret_str += ((1 == factor) || new_unit_str.isEmpty()) ? QString(unit_str) : new_unit_str;
    ret_str += (new_unit_str.isEmpty()) ? QString(unit_str) : new_unit_str;
    return ret_str;
}

template <typename T> void RangeChecker<T>::set_min_max(T min_v, T max_v)
{
    if(EDGE_INFINITE == low_edge || EDGE_INFINITE == up_edge || min_v <= max_v)
    {
        min = min_v;
        max = max_v;
    }
}

template <typename T> void RangeChecker<T>::set_edge(range_edge_enum_t low_e, range_edge_enum_t up_e)
{
    low_edge = low_e; up_edge = up_e;
}

template <typename T> void RangeChecker<T>::set_unit_str(QString unit_s)
{
    unit_str = unit_s;
}

template class RangeChecker<int>;
template class RangeChecker<float>;
template class RangeChecker<double>;
/*end of RangeChecker------------------------------*/

const char* g_prop_name_def_color = "def_color";
const char* g_prop_name_def_font = "def_font";
/*the following two macros MUST be used as a pair.*/
#define SAVE_DEF_COLOR_FONT(ctrl) \
{\
    QColor def_color;\
    QFont def_font;\
    QVariant var_prop;\
    var_prop = (ctrl)->property(g_prop_name_def_color);\
    if(!(var_prop.isValid() && (def_color = var_prop.value<QColor>()).isValid()))\
    {\
        def_color = (ctrl)->textColor();\
    }\
    var_prop = (ctrl)->property(g_prop_name_def_font);\
    def_font = var_prop.isValid() ? var_prop.value<QFont>() : (ctrl)->currentFont();

#define RESTORE_DEF_COLOR_FONT(ctrl) \
    (ctrl)->setTextColor(def_color);\
    (ctrl)->setCurrentFont(def_font);\
}

void append_str_with_color_and_weight(QTextEdit* ctrl, QString str,
                                      QColor new_color, int new_font_weight)
{
    if(!ctrl) return;

    ctrl->moveCursor(QTextCursor::End);

    SAVE_DEF_COLOR_FONT(ctrl);

    QFont new_font = def_font;
    if(!new_color.isValid()) new_color = def_color;
    if(new_font_weight > 0) new_font.setWeight(new_font_weight);

    ctrl->setTextColor(new_color);
    ctrl->setCurrentFont(new_font);
    ctrl->append(str);

    ctrl->moveCursor(QTextCursor::End);

    RESTORE_DEF_COLOR_FONT(ctrl);
}

void append_line_with_styles(QTextEdit* ctrl, str_line_with_styles_t &style_line)
{
    if(!ctrl) return;

    ctrl->moveCursor(QTextCursor::End);

    SAVE_DEF_COLOR_FONT(ctrl);

    QFont new_font = def_font;
    ctrl->insertPlainText("\n");
    for(int idx = 0; idx < style_line.count(); ++idx)
    {
        ctrl->setTextColor(style_line[idx].color);
        new_font.setWeight(style_line[idx].weight);
        ctrl->setCurrentFont(new_font);
        ctrl->insertPlainText(style_line[idx].str);
        ctrl->moveCursor(QTextCursor::End);
    }

    RESTORE_DEF_COLOR_FONT(ctrl);
}

template<typename T> int count_discrete_steps_T(T low_edge, T up_edge, T step)
{
    if(low_edge == up_edge) return 1;
    if(0 == step) return 0;
    if((low_edge < up_edge && step < 0) || (low_edge > up_edge && step > 0)) return 0;

    int cnt = 1;
    T curr = low_edge;
    while(true)
    {
        ++cnt;
        curr += step;
        if((step > 0 && curr >= up_edge) || (step < 0 && curr <= up_edge)) break;
    }
    return cnt;

    /*
     * we use template function instead of one function of double type parameter for the following reason:
     * if use a single function of double type parameter, and a caller passes in float value, the result
     * may be incorrect due to accurancy differency. e.g.:
     *
     * caller passes in the following parameters of float value:
     * low_edge = 0.5, up_edge = 0.6, step = 0.1. we expect the result is 2.
     * but, since 0.6 and 0.1 can't be accurate enough in float, they are something like 0.6000000238
     * and 0.10000000149011612. when calculated in double, the small differences leads to the
     * final result of 3.
     *
     * due to similiar reason, the following method may also give out incorrect result.
    T tmp = (up_edge - low_edge) / step;
    if(tmp < 0) return 0;
    return qCeil(tmp) + 1;
    */
}

int count_discrete_steps(double low_edge, double up_edge, double step)
{
    return count_discrete_steps_T<double>(low_edge, up_edge, step);
}

int count_discrete_steps(float low_edge, float up_edge, float step)
{
    return count_discrete_steps_T<float>(low_edge, up_edge, step);
}

int count_discrete_steps(int low_edge, int up_edge, int step)
{
    return count_discrete_steps_T<int>(low_edge, up_edge, step);
}

CToolKeyFilter::CToolKeyFilter(QObject* obj, QObject * parent)
    :QObject{parent}, m_cared_obj(obj)
{}

CToolKeyFilter::~CToolKeyFilter()
{
    m_keys_to_filter.clear();
}

void CToolKeyFilter::add_keys_to_filter(Qt::Key key)
{
    m_keys_to_filter.insert(key);
}

void CToolKeyFilter::add_keys_to_filter(const QSet<Qt::Key> & keys)
{
    m_keys_to_filter.unite(keys);
}

bool CToolKeyFilter::eventFilter(QObject * obj, QEvent * evt)
{
    if(!obj || !evt || obj != m_cared_obj) return false;

    if(evt->type() == QEvent::KeyPress)
    {
        QKeyEvent * key_evt = static_cast<QKeyEvent *>(evt);
        if(m_keys_to_filter.contains((Qt::Key)key_evt->key()))
        {
            return true;
        }
    }
    return QObject::eventFilter(obj, evt);
}


QImage convertGrayscale16To8(const QImage &img16, pixel_mmpairt_s_t *mmpair, const QRect area, QColor bg)
{
    bool opt_flag = false;

    if (img16.format() != QImage::Format_Grayscale16)
    {
        qWarning("Input image is not Grayscale16");
        return QImage();
    }

    QRect work_area = area;
    if(work_area.isNull())
    {
        work_area.setRect(0, 0, img16.width(), img16.height());
    }
    if(work_area == QRect(0, 0, img16.width(), img16.height()))
    {
        opt_flag = true;
    }

    int x_s = work_area.left(), x_e = x_s + work_area.width();
    int y_s = work_area.top(), y_e = y_s + work_area.height();

    int w = img16.width();
    int h = img16.height();
    QImage img8(w, h, QImage::Format_Grayscale8);

    // 找最小和最大像素值
    quint16 minV = 0xFFFF;
    quint16 maxV = 0;

    if(mmpair)
    {
        minV = mmpair->min_v;
        maxV = mmpair->max_v;
    }
    else
    {
        for (int y = y_s; y < y_e; ++y)
        {
            const quint16 *line16 = reinterpret_cast<const quint16 *>(img16.constScanLine(y));
            for (int x = x_s; x < x_e; ++x)
            {
                quint16 v = line16[x];
                if (v < minV) minV = v;
                if (v > maxV) maxV = v;
            }
        }
    }

    if(minV == maxV)
    {
        // 所有像素一样，直接将workarea置为0或255
        quint8 val = (minV > 0) ? 255 : 0 ;
        if(opt_flag)
        {
            //workarea is the whole img area.
            img8.fill(val);
        }
        else
        {
            if(bg.isValid())img8.fill(bg.rgb());
            quint8 * line8;
            for(int y = y_s; y < y_e ; ++y)
            {
                line8 = reinterpret_cast<quint8 *>(img8.scanLine(y));
                memset(&line8[x_s], val, work_area.width());
            }
        }
        return img8;
    }

    // 映射像素
    for (int y = 0; y < h; ++y)
    {
        const quint16 *line16 = reinterpret_cast<const quint16 *>(img16.constScanLine(y));
        uchar *line8 = img8.scanLine(y);
        for (int x = 0; x < w; ++x)
        {
            quint16 v = line16[x];
            if(work_area.contains(x, y))
            {
                line8[x] = (v - minV) * 255 / (maxV - minV);
            }
            else
            {
                line8[x] = (quint8)v;
            }
        }
    }

    return img8;
}

const gray_pixel_data_type g_12bitpx_max_v = 4095;

const qint64 g_Byte_unit = 1;
const qint64 g_KB_unit = 1024;
const qint64 g_MB_unit = g_KB_unit * 1024;
const qint64 g_GB_unit = g_MB_unit * 1024;
const qint64 g_TB_unit = g_GB_unit * 1024;
void get_total_storage_amount(storage_space_info_s_t &storage_info)
{
    storage_info.total = storage_info.total_used = storage_info.total_ava = 0;
    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &storage : volumes)
    {
        storage_info.total += storage.bytesTotal();
        storage_info.total_ava += storage.bytesAvailable();
    }
    storage_info.total_used = storage_info.total - storage_info.total_ava;
}

QString trans_bytes_cnt_unit(qint64 cnt, qint64 *unit)
{
    QString unit_str;
    qint64 unit_val;

    if(cnt < g_KB_unit)
    {
        unit_str = g_str_Byte;
        unit_val = g_Byte_unit;
    }
    else if(cnt <= g_MB_unit)
    {
        unit_str = g_str_KB;
        unit_val = g_KB_unit;
    }
    else if(cnt <= g_GB_unit)
    {
        unit_str = g_str_MB;
        unit_val = g_MB_unit;
    }
    else if(cnt <= g_TB_unit)
    {
        unit_str = g_str_GB;
        unit_val = g_GB_unit;
    }
    else
    {
        unit_str = g_str_TB;
        unit_val = g_TB_unit;
    }
    if(unit) *unit = unit_val;
    return unit_str;
}
