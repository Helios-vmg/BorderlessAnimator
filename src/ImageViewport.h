/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include "Quadrangular.h"
#include "Settings.h"
#include <chrono>
#include <QLabel>
#include <QImage>
#include <QMatrix>
#include <QTimer>

class LoadedGraphics;

class ImageViewport : public QLabel
{
	Q_OBJECT
	std::unique_ptr<LoadedGraphics> image;
	QPointF origin;
	QPointF translation;
	double rotation = 0;
	double zoom = 1;
	QSize image_size;
	bool update_transform = false;
	QMatrix transform;
	
	std::string name;

	QPointF move_src, move_dst;
	std::chrono::time_point<std::chrono::high_resolution_clock> move_t0;
	double move_duration;
	std::unique_ptr<QTimer> move_timer;
	QMetaObject::Connection move_connection;

	double rotate_rotation0;
	double rotate_speed;
	std::chrono::time_point<std::chrono::high_resolution_clock> rotate_t0;
	std::unique_ptr<QTimer> rotate_timer;
	QMetaObject::Connection rotate_connection;


	QMatrix get_transform(){
		if (!this->update_transform)
			return this->transform;
		this->update_transform = false;
		auto first = QMatrix().translate(-this->origin.x(), -this->origin.y());
		auto second = QMatrix().rotate(this->rotation).scale(this->zoom, this->zoom);;
		return this->transform = first * second * QMatrix().translate(this->translation.x(), this->translation.y());
	}
public:
	explicit ImageViewport(QWidget *parent = 0);
	explicit ImageViewport(std::string &&name, const QSize &size, QWidget *parent = 0);
	QSize get_image_size() const{
		return this->image_size;
	}
	void flip(bool hor);

	void paintEvent(QPaintEvent *) override;
	void set_image(std::unique_ptr<LoadedGraphics> &&li, const QSize &geom);
	const std::string &get_name() const{
		return this->name;
	}

	void move_by_command(const QPointF &);
	void set_scale(double scale);
	void set_origin(int x, int y);
	double get_rotation() const{
		return this->rotation;
	}
	void set_rotation(double theta);
	void anim_move(int x, int y, double speed);
	void anim_rotate(double speed);

signals:
	void transform_updated();

public slots:
	void move_timeout();
	void rotate_timeout();
};

#endif // IMAGEVIEWPORT_H
