#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QDebug>

class ClickableLabel : public QLabel
{
 public:
    explicit ClickableLabel( QWidget * parent = 0 );
    ~ClickableLabel();

 signals:
    void clicked();

 protected:
    void mousePressEvent ( QMouseEvent * event ) ;
};

#endif // CLICKABLELABEL_H
