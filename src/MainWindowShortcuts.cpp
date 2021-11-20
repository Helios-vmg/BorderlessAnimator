/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"

void MainWindow::quit_slot(){
	this->app->quit();
}

void MainWindow::background_swap_slot(){
	this->window_state->flip_using_checkerboard_pattern();
	this->set_background();
}

void MainWindow::close_slot(){
	this->app->postEvent(this, new QCloseEvent());
}

void MainWindow::zoom_in_slot(){
	if (!this->displayed_image)
		return;
	this->change_zoom(true);
}

void MainWindow::zoom_out_slot(){
	if (!this->displayed_image)
		return;
	this->change_zoom(false);
}

void MainWindow::up_slot(){
	this->offset_image(QPoint(0, this->window_state->get_movement_size()));
}

void MainWindow::down_slot(){
	this->offset_image(QPoint(0, -this->window_state->get_movement_size()));
}

void MainWindow::left_slot(){
	this->offset_image(QPoint(this->window_state->get_movement_size(), 0));
}

void MainWindow::right_slot(){
	this->offset_image(QPoint(-this->window_state->get_movement_size(), 0));
}

void MainWindow::up_big_slot(){
	auto size = this->size().height() / 2;
	this->offset_image(QPoint(0, size));
}

void MainWindow::down_big_slot(){
	auto size = this->size().height() / 2;
	this->offset_image(QPoint(0, -size));
}

void MainWindow::left_big_slot(){
	auto size = this->size().width() / 2;
	this->offset_image(QPoint(size, 0));
}

void MainWindow::right_big_slot(){
	auto size = this->size().width() / 2;
	this->offset_image(QPoint(-size, 0));
}

void MainWindow::offset_image(const QPoint &offset){
	this->move_image(this->ui->label->pos() + offset);
}

void MainWindow::reset_zoom_slot(){
	if (!this->displayed_image)
		return;
	int zoom = this->get_current_zoom();
	this->set_current_zoom_mode(ZoomMode::Normal);
	this->ui->label->reset_transform();
	this->set_zoom();
	this->apply_zoom(false, zoom);
}

void cycle_zoom_mode(ZoomMode &mode){
	switch (mode){
		case ZoomMode::Normal:
		case ZoomMode::Locked:
			mode = ZoomMode::AutoFit;
			break;
		case ZoomMode::AutoFit:
		case ZoomMode::AutoRotFit:
			mode = ZoomMode::AutoFill;
			break;
		case ZoomMode::AutoFill:
		case ZoomMode::AutoRotFill:
			mode = ZoomMode::Normal;
			break;
	}
}

void MainWindow::cycle_zoom_mode_slot(){
	if (!this->displayed_image)
		return;
	auto mode = this->get_current_zoom_mode();
	cycle_zoom_mode(mode);
	this->set_current_zoom_mode(mode);
	auto zoom = this->get_current_zoom();
	this->set_zoom();
	this->apply_zoom(false, zoom);
	this->set_background_sizes();
}

void toggle_lock_zoom(ZoomMode &mode){
	mode = mode == ZoomMode::Locked ? ZoomMode::Normal : ZoomMode::Locked;
}

void MainWindow::toggle_lock_zoom_slot(){
	if (!this->displayed_image)
		return;
	auto mode = this->window_state->get_zoom_mode();
	toggle_lock_zoom(mode);
	this->window_state->set_zoom_mode(mode);
}

void MainWindow::toggle_fullscreen(){
	auto zoom = this->get_current_zoom();
	this->window_state->set_fullscreen(!this->window_state->get_fullscreen());
	if (!this->window_state->get_fullscreen()){
		if (this->displayed_image)
			this->apply_zoom(false, zoom);
		this->setGeometry(this->window_rect);
		this->restore_image_pos();
	}else{
		this->save_image_pos(true);
		if (this->displayed_image){
			this->set_zoom();
			this->apply_zoom(false, zoom);
		}
		this->resolution_to_window_size();
		this->reposition_image();
		//this->move_image(QPoint(0, 0));
	}
	this->set_background_sizes();
}

void MainWindow::rotate(bool right, bool fine){
	if (this->current_zoom_mode_is_auto_rotation())
		this->set_current_zoom_mode(disable_flag(this->get_current_zoom_mode(), ZoomMode::AutomaticRotation));
	this->ui->label->rotate((right ? 1 : -1) * (fine ? 1 : 90));
	this->fix_positions_and_zoom();
	//this->reposition_image();
	//this->ui->label->repaint();
}

void MainWindow::rotate_left(){
	this->rotate(false);
}

void MainWindow::rotate_right(){
	this->rotate(true);
}

void MainWindow::rotate_left_fine(){
	this->rotate(false, true);
}

void MainWindow::rotate_right_fine(){
	this->rotate(true, true);
}

void MainWindow::minimize_slot(){
	this->setWindowState(Qt::WindowMinimized);
}

void MainWindow::minimize_all_slot(){
	this->app->minimize_all();
}

void MainWindow::flip_h(){
	this->ui->label->flip(true);
	this->ui->label->repaint();
}

void MainWindow::flip_v(){
	this->ui->label->flip(false);
	this->ui->label->repaint();
}
