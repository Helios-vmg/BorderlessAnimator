/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "LoadedImage.h"
#include "DirectoryListing.h"
#include "ImageViewerApplication.h"
#include "Misc.h"
#include "Settings.h"
#include "ImageViewport.h"
#include <QMainWindow>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QStringList>
#include <QShortcut>
#include <vector>
#include <memory>
#include <chrono>

namespace Ui {
class MainWindow;
}

class MouseEvent{
public:
	bool left;
	bool right;
	bool middle;
	int button_sum;
	QPoint absolute;
	QPoint relative;
	MouseEvent(const QMouseEvent &);
	MouseEvent(const MouseEvent &) = default;
	MouseEvent &operator=(const MouseEvent &) = default;
};

class MainWindow : public QMainWindow{
	Q_OBJECT

public:
	typedef std::shared_ptr<ImageViewport> sharedp_t;
protected:
	std::shared_ptr<Ui::MainWindow> ui;
	ImageViewerApplication *app;
	QPoint origin;
	std::vector<QRect> desktop_sizes,
		screen_sizes;
	int current_desktop = -1;
	QRect window_rect;
	QPoint first_mouse_pos,
		first_window_pos,
		first_label_pos;
	Optional<QPoint> image_pos;
	QSize first_window_size;
	std::vector<int> horizontal_clampers,
		vertical_clampers;
	//QString current_directory,
	//	current_filename;
	std::vector<std::shared_ptr<QShortcut>> shortcuts;
	bool not_moved;

	std::map<std::string, sharedp_t> windows_by_name;

	enum class ResizeMode{
		None        = 0,
		Top         = 1 << 1,
		Right       = 1 << 2,
		Bottom      = 1 << 3,
		Left        = 1 << 4,
		TopLeft     = ResizeMode::Top | ResizeMode::Left,
		TopRight    = ResizeMode::Top | ResizeMode::Right,
		BottomRight = ResizeMode::Bottom | ResizeMode::Right,
		BottomLeft  = ResizeMode::Bottom | ResizeMode::Left,
	};
	ResizeMode resize_mode;

	std::shared_ptr<WindowState> window_state;

	bool move_image(const QPoint &new_position);
	QPoint compute_movement(const QPoint &new_position, const QPoint &mouse_position);
	bool compute_resize(QPoint &out_label_pos, QRect &out_window_rect, QPoint mouse_offset, const QPoint &mouse_position);
	void move_window(const QPoint &new_position, const QPoint &mouse_position);
	void reset_settings();
	ResizeMode get_resize_mode(const QPoint &pos);
	void set_resize_mode(const QPoint &pos);
	bool perform_clamping();
	bool force_keep_window_in_desktop();
	void init(bool restoring);
	void show_context_menu(QMouseEvent *);
	void offset_image(const QPoint &);
	int get_current_desktop_number();
	void set_current_desktop_and_fix_positions_by_window_position(int old_desktop);
	void set_desktop_size();
	void set_desktop_size(int screen);
	double get_current_zoom() const;
	void set_current_zoom(double);
	void set_current_zoom_mode(const ZoomMode &);
	ZoomMode get_current_zoom_mode() const;
	void resolution_to_window_size();
	void reposition_image();
	void clear_image_pos();
	void rotate(bool right, bool fine = false);

	struct ZoomResult{
		double zoom;
		QSize label_size;
	};
	
protected:
	void mousePressEvent(QMouseEvent *ev) override;
	void mouseReleaseEvent(QMouseEvent *ev) override;
	void mouseMoveEvent(QMouseEvent *ev) override;
	void reset_left_mouse(const MouseEvent &);
	bool set_cursor_flags(const MouseEvent &);
	//void keyPressEvent(QKeyEvent *ev) override;
	//void keyReleaseEvent(QKeyEvent *ev) override;
	void resizeEvent(QResizeEvent *ev) override;
	void changeEvent(QEvent *ev) override;
	void closeEvent(QCloseEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *) override;
	//bool event(QEvent *) override;

public:
	explicit MainWindow(ImageViewerApplication &app, const QRect &geom, QWidget *parent = 0);
	void resolution_change(int screen);
	void work_area_change(int screen);
	void resize_window_rect(const QSize &);
	void move_window_rect(const QPoint &);
	void set_window_rect(const QRect &);
	double get_image_zoom() const;
	void build_context_menu(QMenu &main_menu);
	bool current_zoom_mode_is_auto() const{
		return check_flag(this->get_current_zoom_mode(), ZoomMode::AutomaticZoom);
	}
	bool current_zoom_mode_is_auto_rotation() const{
		return check_flag(this->get_current_zoom_mode(), ZoomMode::AutomaticRotation);
	}
	ImageViewerApplication &get_app(){
		return *this->app;
	}
	void load(const QString &path, std::string &&name);
	sharedp_t get_window(const std::string &name);

public slots:
	void quit_slot();
	void background_swap_slot();
	void close_slot();
	void zoom_in_slot();
	void zoom_out_slot();
	void reset_zoom_slot();
	void up_slot();
	void down_slot();
	void left_slot();
	void right_slot();
	void up_big_slot();
	void down_big_slot();
	void left_big_slot();
	void right_big_slot();
	void cycle_zoom_mode_slot();
	void set_zoom();
	void toggle_lock_zoom_slot();
	void toggle_fullscreen();
	void rotate_left();
	void rotate_right();
	void rotate_left_fine();
	void rotate_right_fine();
	void minimize_slot();
	void minimize_all_slot();
	void flip_h();
	void flip_v();

signals:
	void closing(MainWindow *);

};

#endif // MAINWINDOW_H
