/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Misc.h"
#include <algorithm>
#include <limits>
#include <QImage>
#include <QMetaEnum>
#include <QDir>
#include <exception>
#include <cassert>
#include "GenericException.h"

MainWindow::MainWindow(ImageViewerApplication &app, const QRect &geom, QWidget *parent):
		QMainWindow(parent),
		ui(new Ui::MainWindow),
		app(&app){
	this->init(false);
	this->setGeometry(geom);
	this->origin = -geom.topLeft();
	//this->open_path_and_display_image(path);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->show();
}

void MainWindow::init(bool restoring){
	if (!this->window_state)
		this->window_state = std::make_shared<WindowState>();
	this->not_moved = false;
	this->window_state->set_zoom(1);
	this->window_state->set_fullscreen_zoom(1);
	this->window_state->set_fullscreen(false);
	this->ui->setupUi(this);
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
	this->reset_settings();

	this->set_desktop_size();
	assert(this->desktop_sizes.size());
	if (restoring){
		auto index = this->app->desktop()->screenNumber(this->pos());
		if (index < 0 || index >= this->desktop_sizes.size())
			index = 0;
		this->current_desktop = index;
		this->move(this->desktop_sizes[index].topLeft());
	}else{
		auto index = this->app->desktop()->screenNumber(QCursor::pos());
		if (index < 0 || index >= this->desktop_sizes.size())
			index = 0;
		this->current_desktop = index;
		auto pos = this->desktop_sizes[index].topLeft();
		this->move(pos);
		this->window_rect.moveTopLeft(pos);
	}

	this->setMouseTracking(true);
	this->ui->centralWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
}

int MainWindow::get_current_desktop_number(){
	auto ret = this->app->desktop()->screenNumber(this->pos());
	if (ret < 0)
		ret = 0;
	return ret;
}

void MainWindow::set_current_desktop_and_fix_positions_by_window_position(int old_desktop){
	this->current_desktop = this->get_current_desktop_number();
}

void MainWindow::set_desktop_size(){
	auto n = this->app->desktop()->screenCount();
	for (decltype(n) i = 0; i < n; i++)
		this->set_desktop_size(i);
}

void MainWindow::set_desktop_size(int screen){
	assert(screen >= 0);
	assert(this->desktop_sizes.size() == this->screen_sizes.size());
	if (this->desktop_sizes.size() <= screen){
		this->desktop_sizes.resize(screen + 1);
		this->screen_sizes.resize(screen + 1);
	}
	this->desktop_sizes[screen] = this->app->desktop()->availableGeometry(screen);
	this->desktop_sizes[screen].setHeight(this->desktop_sizes[screen].height()); //???
	this->screen_sizes[screen] = this->app->desktop()->screenGeometry(screen);
}

void MainWindow::clear_image_pos(){
	qDebug() << "Clearing image position.";
	this->image_pos.clear();
}

double ratio(const QSize &size){
	return (double)size.width() / (double)size.height();
}

int area(const QSize &size){
	return size.width() * size.height();
}

#define IS_WIDER >

void MainWindow::set_zoom(){
}

double MainWindow::get_current_zoom() const{
	return !this->window_state->get_fullscreen() ? this->window_state->get_zoom() : this->window_state->get_fullscreen_zoom();
}

void MainWindow::set_current_zoom(double value){
	if (this->window_state->get_fullscreen())
		this->window_state->set_fullscreen_zoom(value);
	else
		this->window_state->set_zoom(value);
}

void MainWindow::set_current_zoom_mode(const ZoomMode &mode){
	if (!this->window_state->get_fullscreen())
		this->window_state->set_zoom_mode(mode);
	else
		this->window_state->set_fullscreen_zoom_mode(mode);
}

ZoomMode MainWindow::get_current_zoom_mode() const{
	return !this->window_state->get_fullscreen() ? this->window_state->get_zoom_mode() : this->window_state->get_fullscreen_zoom_mode();
}

class ElapsedTimer{
	QString task;
	clock_t t0;
public:
	ElapsedTimer(const QString &task): task(task){
		qDebug() << "Task " << this->task << " started.";
		this->t0 = clock();
	}
	~ElapsedTimer(){
		auto t1 = clock();
		qDebug() << "Task " << this->task << " took " << (t1 - t0) / (double)CLOCKS_PER_SEC << " seconds.";
	}
};

void MainWindow::show_context_menu(QMouseEvent *ev){
	this->app->postEvent(this, new QContextMenuEvent(QContextMenuEvent::Other, ev->screenPos().toPoint()));
}

void MainWindow::build_context_menu(QMenu &main_menu){
	main_menu.addAction("Close", this, SLOT(close_slot()));
}

//void MainWindow::keyReleaseEvent(QKeyEvent *ev){
//	QMainWindow::keyReleaseEvent(ev);
//}

void MainWindow::resizeEvent(QResizeEvent *){}

void MainWindow::changeEvent(QEvent *ev){
	if (ev->type() == QEvent::WindowStateChange){
		auto &event = dynamic_cast<QWindowStateChangeEvent &>(*ev);
		if (this->isMaximized())
			this->setWindowState(event.oldState());
		else
			ev->accept();
	}else
		ev->accept();
}

void MainWindow::closeEvent(QCloseEvent *){
	emit closing(this);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *ev){
	if (ev->reason() != QContextMenuEvent::Other)
		return;
	this->not_moved = false;
	auto menu = this->app->build_context_menu(this);
	menu->move(ev->pos());
	menu->exec();
}

void MainWindow::resolution_change(int screen){
	auto desktop = this->app->desktop();
	int this_screen = desktop->screenNumber(this);
	if (screen != this_screen)
		return;
	this->set_desktop_size(this_screen);
}

void MainWindow::resolution_to_window_size(){
	this->setGeometry(this->screen_sizes[this->get_current_desktop_number()]);
}

void MainWindow::work_area_change(int screen){
	auto desktop = this->app->desktop();
	int this_screen = desktop->screenNumber(this);
	if (screen != this_screen)
		return;
	this->set_desktop_size(this_screen);
}

void MainWindow::resize_window_rect(const QSize &s){
	this->window_rect.setSize(s);
	if (!this->window_state->get_fullscreen())
		this->resize(s);
}

void MainWindow::move_window_rect(const QPoint &p){
	this->window_rect.setX(p.x());
	this->window_rect.setY(p.y());
	if (!this->window_state->get_fullscreen())
		this->move(p);
	this->set_current_desktop_and_fix_positions_by_window_position(this->current_desktop);
}

void MainWindow::set_window_rect(const QRect &r){
	this->window_rect = r;
	if (!this->window_state->get_fullscreen())
		this->setGeometry(r);
	this->set_current_desktop_and_fix_positions_by_window_position(this->current_desktop);
}

double MainWindow::get_image_zoom() const{
	return this->window_state->get_zoom();
}

void MainWindow::load(const QString &path, std::string &&name){
	auto image = LoadedImage::create(*this->app, path);
	if (!image)
		return;
	auto geometry = this->geometry();
	auto viewport = std::make_shared<ImageViewport>(std::move(name), geometry.size(), this/*->ui->centralWidget*/);
	//this->ui->label->set_image(LoadedImage::create(*this->app, path));
	viewport->set_image(std::move(image), geometry.size());
	viewport->show();
	this->windows_by_name[viewport->get_name()] = viewport;
}

MainWindow::sharedp_t MainWindow::get_window(const std::string &name){
	auto it = this->windows_by_name.find(name);
	if (it == this->windows_by_name.end())
		return {};
	return it->second;
}
