/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"
#include "MainWindow.h"
#include "Script.h"
#include <sstream>

void ImageViewerApplication::handle_load(const QStringList &args){
	if (args.size() < 4)
		return;
	this->main_window->load(args[2], args[3].toStdString());
}

void ImageViewerApplication::handle_scale(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	auto scale = expect_real(args[3]);
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->set_scale(scale);
}

void ImageViewerApplication::handle_setorigin(const QStringList &args){
	if (args.size() < 5)
		return;
	auto name = args[2].toStdString();
	auto x = expect_integer(args[3]);
	auto y = expect_integer(args[4]);
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->set_origin(x, y);
}

void ImageViewerApplication::handle_move(const QStringList &args){
	if (args.size() < 5)
		return;
	auto name = args[2].toStdString();
	auto x = expect_relabs_integer(args[3]);
	auto y = expect_relabs_integer(args[4]);

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->move_by_command(set(window->get_position(), x, y));
}

void ImageViewerApplication::handle_rotate(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	auto theta = expect_relabs_real(args[3]);

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	auto rotation = window->get_rotation();
	if (theta.second)
		rotation += theta.first;
	else
		rotation = theta.first;
	window->set_rotation(rotation);
}

void ImageViewerApplication::handle_animmove(const QStringList &args){
	if (args.size() < 6)
		return;
	auto name = args[2].toStdString();
	auto x = expect_integer(args[3]);
	auto y = expect_integer(args[4]);
	auto speed = expect_real(args[5]);

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->anim_move(x, y, speed);
}

void ImageViewerApplication::handle_animrotate(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	auto speed = expect_real(args[3]);

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->anim_rotate(speed);
}

void ImageViewerApplication::handle_fliph(const QStringList &args){
	if (args.size() < 2)
		return;
	auto name = args[2].toStdString();
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->fliph();
}

void ImageViewerApplication::handle_flipv(const QStringList &args){
	if (args.size() < 3)
		return;
	auto name = args[2].toStdString();
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->flipv();
}

void ImageViewerApplication::handle_loadscript(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	auto &path = args[3];
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->load_script(path);
}
