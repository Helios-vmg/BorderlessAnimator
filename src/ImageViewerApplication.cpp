/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"
#include "MainWindow.h"
#include "Misc.h"
#include "GenericException.h"
#include <QShortcut>
#include <QMessageBox>
#include <sstream>
#include <cassert>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <random>
#include <QJsonDocument>

template <typename T>
class AutoSetter{
	T *dst;
	T old_value;
public:
	AutoSetter(): dst(nullptr){}
	AutoSetter(T &dst, T new_value): dst(&dst), old_value(dst){
		*this->dst = new_value;
	}
	AutoSetter(const AutoSetter &) = delete;
	AutoSetter &operator=(const AutoSetter &) = delete;
	AutoSetter(AutoSetter &&other){
		*this = std::move(other);
	}
	AutoSetter &operator=(AutoSetter &&other){
		this->dst = other.dst;
		other.dst = nullptr;
		this->old_value = std::move(other.old_value);
		return *this;
	}
	~AutoSetter(){
		if (this->dst)
			*this->dst = this->old_value;
	}
};

template <typename T>
AutoSetter<T> autoset(T &dst, T value){
	return AutoSetter<T>(dst, value);
}

ImageViewerApplication::ImageViewerApplication(int &argc, char **argv, const QString &unique_name):
		SingleInstanceApplication(argc, argv, unique_name),
		tray_icon(QIcon(":/icon16.png"), this){
	QDir::setCurrent(this->applicationDirPath());
	this->reset_tray_menu();
	this->tray_icon.show();
	this->setQuitOnLastWindowClosed(false);
	ImageViewerApplication::new_instance(this->args);
	this->setup_slots();
}

ImageViewerApplication::~ImageViewerApplication(){
	this->windows.clear();
}

void ImageViewerApplication::new_instance(const QStringList &args){
	auto p = std::make_shared<MainWindow>(*this, args);
	if (!p->is_null())
		this->add_window(p);
}

void ImageViewerApplication::add_window(sharedp_t p){
	if (!p->is_loaded()){
		p->close();
		return;
	}
	connect(p.get(), SIGNAL(closing(MainWindow *)), this, SLOT(window_closing(MainWindow *)));
	p->show();
	p->raise();
	p->activateWindow();
	p->setFocus();
	this->windows[(uintptr_t)p.get()] = p;
}

void ImageViewerApplication::window_closing(MainWindow *window){
	auto it = this->windows.find((uintptr_t)window);
	if (it == this->windows.end())
		return;
	this->windows.erase(it);
}

void ImageViewerApplication::save_current_windows(std::vector<std::shared_ptr<WindowState>> &windows){
	windows.clear();
	windows.reserve(this->windows.size());
	for (auto &w : this->windows)
		windows.push_back(w.second->save_state());
}

class SettingsException : public GenericException{
public:
	SettingsException(const char *what) : GenericException(what){}
};

QByteArray hash_file(const QByteArray &contents){
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(contents);
	return hash.result();
}

void ImageViewerApplication::conditionally_save_file(const QByteArray &contents, const QString &path, QByteArray &last_digest){
	auto new_digest = hash_file(contents);
	if (!last_digest.isNull() && new_digest == last_digest)
		return;

	QFile file(path);
	file.open(QFile::WriteOnly | QFile::Truncate);
	if (!file.isOpen())
		return;
	file.write(contents);

	last_digest = new_digest;
}

void ImageViewerApplication::restore_current_windows(const std::vector<std::shared_ptr<WindowState>> &window_states){
	for (auto &state : window_states)
		this->add_window(std::make_shared<MainWindow>(*this, state));
}

std::shared_ptr<QMenu> ImageViewerApplication::build_context_menu(MainWindow *caller){
	this->context_menu_last_requester = caller;
	std::shared_ptr<QMenu> ret(new QMenu);
	auto initial = ret->actions().size();
	if (caller){
		caller->build_context_menu(*ret);
		if (ret->actions().size() != initial)
			ret->addSeparator();
	}
	auto receiver = caller ? (QObject *)caller : (QObject *)this;
	ret->addAction("Quit", receiver, SLOT(quit_slot()));
	return ret;
}

QJsonDocument ImageViewerApplication::load_json(const QString &path, QByteArray &digest){
	if (path.isNull())
		return {};
	QFile file(path);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
		return {};
	auto contents = file.readAll();
	digest = hash_file(contents);
	return QJsonDocument::fromJson(contents);
}

void ImageViewerApplication::resolution_change(int screen){
	for (auto &i : this->windows)
		i.second->resolution_change(screen);
}

void ImageViewerApplication::work_area_change(int screen){
	for (auto &i : this->windows)
		i.second->work_area_change(screen);
}

void ImageViewerApplication::minimize_all(){
	for (auto &p : this->windows)
		p.second->minimize_slot();
}

QString get_config_location(bool strip_last_item = true){
	QString ret;
	auto list = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	if (!list.size())
		return QString::null;
	ret = list[0];
	if (strip_last_item){
		int index = ret.lastIndexOf('/');
		if (index < 0)
			index = ret.lastIndexOf('\\');
		ret = ret.mid(0, index);
	}
	auto c = QDir::separator();
	ret += c;
	ret += "BorderlessAnimator";
	ret += c;
	QDir dir(ret);
	if (!dir.mkpath(ret))
		return QString::null;
	return ret;
}

QString ImageViewerApplication::get_config_location(){
	if (this->config_location.isNull())
		this->config_location = ::get_config_location();
	return this->config_location;
}

QString ImageViewerApplication::get_config_subpath(QString &dst, const char *sub){
	auto &ret = dst;
	if (ret.isNull()){
		ret = this->get_config_location();
		if (ret.isNull())
			return ret;
		ret += sub;
	}
	return ret;
}

QString ImageViewerApplication::get_settings_filename(){
	return this->get_config_subpath(this->config_filename, "settings.json");
}

QString ImageViewerApplication::get_state_filename(){
	return this->get_config_subpath(this->state_filename, "state.json");
}

void ImageViewerApplication::setup_slots(){
	connect(this->desktop(), SIGNAL(resized(int)), this, SLOT(resolution_change(int)));
	connect(this->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(work_area_change(int)));
}

void ImageViewerApplication::reset_tray_menu(){
	this->last_tray_context_menu = this->build_context_menu();
	this->tray_icon.setContextMenu(this->last_tray_context_menu.get());
	this->tray_context_menu.swap(this->last_tray_context_menu);
}

QImage ImageViewerApplication::load_image(const QString &path){
	return QImage(path);
}

QString generate_random_string(){
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> dist1('a', 'z');
	std::uniform_int_distribution<int> dist2(0, 1);
	QString ret;
	for (int i = 16; i--;){
		int c = dist1(rng);
		if (dist2(rng))
			c = toupper(c);
		ret += (char)c;
	}
	return ret;
}

std::unique_ptr<QMovie> ImageViewerApplication::load_animation(const QString &path){
	return std::make_unique<QMovie>(path);
}

bool ImageViewerApplication::is_animation(const QString &path){
	return path.endsWith(".gif", Qt::CaseInsensitive);
}

QString get_per_user_unique_id(){
	auto location = get_config_location(false);
	if (location == QString::null)
		return "";
	location += "user_id.txt";
	QFile file(location);
	if (!file.exists()){
		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
			return "";
		auto ret = generate_random_string();
		auto bytes = ret.toUtf8();
		file.write(bytes.data(), bytes.size());
		return ret;
	}
	if (!file.open(QIODevice::ReadOnly))
		return "";
	auto bytes = file.readAll();
	if (bytes.isNull())
		return "";
	return QString::fromUtf8(bytes);
}

void ImageViewerApplication::turn_transparent(MainWindow &window, bool yes){
	auto state = window.save_state();
	std::shared_ptr<MainWindow> new_window;
	if (yes)
		new_window = std::make_shared<TransparentMainWindow>(*this, state);
	else
		new_window = std::make_shared<MainWindow>(*this, state);
	this->add_window(new_window);
}
