#ifndef EASHAPE_H
#define EASHAPE_H

#include <QPainter>
#include <Eigen/Dense>

class EaShape {

public:
    virtual bool onDrag(double x, double y) {
        return true;
    }

    virtual void onDraw(QPainter* painter) = 0;

protected:
    Eigen::Matrix<double, 3, 1, Eigen::DontAlign> lcursor;
    Eigen::Matrix<double, 3, 1, Eigen::DontAlign> ccursor;

};


#endif
