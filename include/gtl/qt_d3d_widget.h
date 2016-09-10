#ifndef URWQWOFVZXBBSWEF_GTL_QT_D3D_WIDGET_H_
#define URWQWOFVZXBBSWEF_GTL_QT_D3D_WIDGET_H_

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)                              
    
    namespace gtl::qt::

    class d3d_widget : QWidget
    
-----------------------------------------------------------------------------*/

#include <QtWidgets/QWidget>
#include <QDebug>
#include <QTimer>

#include <gtl/d3d_types.h>
#include <gtl/stage.h>
#include <gtl/events.h>
#include <thread>

namespace gtl {
namespace qt {

    class d3d_widget : public QWidget {
        gtl::d3d::device dev_;
        gtl::d3d::command_queue cqueue_;
        gtl::d3d::swap_chain swchain_;
        gtl::stage stage_;
        std::unique_ptr<QTimer> trigger_;

    public:
        d3d_widget::d3d_widget(QWidget* parent_)
            : QWidget(parent_),
              dev_{gtl::tags::debug{}},
              cqueue_{dev_},
              swchain_{
                  reinterpret_cast<HWND>(this->winId()), // TODO Qt winId() => HWND ??
                  cqueue_, 3},
              stage_{swchain_, cqueue_, 3}
        {
            setAttribute(Qt::WA_PaintOnScreen, true);
            setAttribute(Qt::WA_NativeWindow, true);

            //this->resize(960,540);
            //this->setVisible(true);
            //this->show();

            stage_.dispatch_event(gtl::event{gtl::events::none{}});

            trigger_ = std::make_unique<QTimer>(this);
            connect(trigger_.get(), &QTimer::timeout, this, &d3d_widget::present);
            trigger_->start();
        }

        void update()
        {
            qDebug() << "update..";
            stage_.dispatch_event(gtl::event{gtl::events::none{}});
        }
        void present()
        {
            stage_.present();
        }

        void paintEvent(QPaintEvent*) final
        {
        }
        QPaintEngine* paintEngine() const final
        {
            return nullptr;
        }
    };
}
} // namespaces
#endif
