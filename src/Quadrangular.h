/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef QUADRANGULAR_H
#define QUADRANGULAR_H

#include <QSize>
#include <QRect>
#include <QPoint>
#include <QMatrix>
#include "Misc.h"

inline qreal norm(const QPointF &p){
	return sqrt(p.x() * p.x() + p.y() * p.y());
}

class Quadrangular{
	QPointF corners[4];
	template <typename T>
	void set_size(const T &size){
		this->corners[0] = QPointF(0, 0);
		this->corners[1] = QPointF(size.width(), 0);
		this->corners[2] = QPointF(size.width(), size.height());
		this->corners[3] = QPointF(0, size.height());
		//this->move(-to_QPointF(size));
	}
	void move(const QPointF &p){
		for (auto &c : this->corners)
			c += p;
	}
public:
	Quadrangular(){
		this->set_size<QSize>({});
	}
	Quadrangular(const QSize &size){
		this->set_size(size);
	}
	Quadrangular(const QSizeF &size){
		this->set_size(size);
	}
	Quadrangular(const QRect &rect){
		this->set_size(rect.size());
		this->move(rect.topLeft());
	}
	Quadrangular(const QRectF &rect){
		this->set_size(rect.size());
		this->move(rect.topLeft());
	}
	Quadrangular(const Quadrangular &) = default;
	Quadrangular &operator=(const Quadrangular &) = default;
	const Quadrangular &operator*=(const QMatrix &transform){
		for (auto &p : this->corners)
			p = transform.map(p);
		return *this;
	}
	template <typename T>
	Quadrangular operator*(const T &x) const{
		auto ret = *this;
		ret *= x;
		return ret;
	}
	const Quadrangular &operator+=(const QPointF &shift){
		for (auto &p : this->corners)
			p += shift;
		return *this;
	}
	template <typename T>
	Quadrangular operator+(const T &x) const{
		auto ret = *this;
		ret += x;
		return ret;
	}
	const Quadrangular &operator-=(const QPointF &shift){
		return *this += -shift;
	}
	template <typename T>
	Quadrangular operator-(const T &x) const{
		auto ret = *this;
		ret -= x;
		return ret;
	}
	QRectF get_bounding_box() const{
		auto minx = this->corners[0].x();
		auto miny = this->corners[0].y();
		auto maxx = this->corners[0].x();
		auto maxy = this->corners[0].y();
		for (auto &p : this->corners){
			set_min(minx, p.x());
			set_min(miny, p.y());
			set_max(maxx, p.x());
			set_max(maxy, p.y());
		}
		return QRectF(minx, miny, maxx - minx, maxy - miny);
	}
	QPointF get_offset() const{
		return -this->get_bounding_box().topLeft();
	}
	QPointF move_to_origin(){
		auto ret = this->get_offset();
		*this += ret;
		return ret;
	}
	QPointF compute_origin(const QPointF &p) const{
		auto w = norm(this->corners[1] - this->corners[0]);
		auto h = norm(this->corners[3] - this->corners[0]);
		
		return this->get_offset() + p.x() / w * this->corners[1] +  p.y() / h * this->corners[3];
	}
};

#endif
