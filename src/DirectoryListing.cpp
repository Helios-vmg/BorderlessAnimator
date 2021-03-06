/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "DirectoryListing.h"
#include "Misc.h"
#include "ImageViewerApplication.h"
#include <QDir>
#include <QtConcurrent/QtConcurrentRun>
#include <QImageReader>
#include <algorithm>

const Qt::CaseSensitivity platform_case = Qt::CaseInsensitive;

static const char *hardcoded_supported_extensions[] = {
	"*.bmp",
	"*.jpg",
	"*.jpeg",
	"*.png",
	"*.gif",
	"*.svg",
	"*.webp",
	"*.ico",
	"*.tga",
	"*.tif",
	"*.tiff",
	//"*.jp2", //TODO: add support
};

enum class ImageSupport{
	Qt = 1,
	ExternalOnly = 2,
	Both = 3,
};

typedef std::map<QString, ImageSupport> supported_extensions_t;
supported_extensions_t supported_extensions;

const supported_extensions_t::iterator &get_iterator(const void *p){
	return *(const supported_extensions_t::iterator *)p;
}

supported_extensions_t::iterator &get_iterator(void *p){
	return *(supported_extensions_t::iterator *)p;
}

const supported_extensions_t::iterator &get_iterator(const std::unique_ptr<void, void(*)(void *)> &p){
	return get_iterator(p.get());
}

supported_extensions_t::iterator &get_iterator(std::unique_ptr<void, void(*)(void *)> &p){
	return get_iterator(p.get());
}

ExtensionIterator::ExtensionIterator(const void *p): pimpl(nullptr, release_pointer){
	this->pimpl.reset(new supported_extensions_t::iterator(get_iterator(p)));
}

ExtensionIterator::ExtensionIterator(const ExtensionIterator &other): ExtensionIterator(other.pimpl.get()){}

const ExtensionIterator &ExtensionIterator::operator=(const ExtensionIterator &other){
	this->pimpl.reset(new supported_extensions_t::iterator(get_iterator(other.pimpl)));
	return *this;
}

void ExtensionIterator::release_pointer(void *p){
	delete (supported_extensions_t::iterator *)p;
}

ExtensionIterator ExtensionIterator::begin(){
	auto i = supported_extensions.begin();
	return ExtensionIterator(&i);
}

ExtensionIterator ExtensionIterator::end(){
	auto i = supported_extensions.end();
	return ExtensionIterator(&i);
}

const ExtensionIterator &ExtensionIterator::operator++(){
	++get_iterator(this->pimpl);
	return *this;
}

bool ExtensionIterator::operator==(const ExtensionIterator &other) const{
	return get_iterator(this->pimpl) == get_iterator(other.pimpl);
}

QString ExtensionIterator::operator*() const{
	return get_iterator(this->pimpl)->first;
}

void initialize_supported_extensions(){
	for (auto p : hardcoded_supported_extensions)
		supported_extensions[p] = ImageSupport::ExternalOnly;
#if 0
	for (auto s : QImageReader::supportedImageFormats()){
		auto s2 = "*." + s;
		auto it = supported_extensions.find(s2);
		if (it != supported_extensions.end())
			it->second = ImageSupport::Both;
		else
			supported_extensions[s2] = ImageSupport::Qt;
	}
#endif
}

template <Qt::CaseSensitivity CS>
bool strcmpci(const QString &a, const QString &b){
	return a.compare(a, b, CS) < 0;
}

QStringList get_local_entries(QString path){
	QDir directory(path);
	directory.setFilter(QDir::Files | QDir::Hidden);
	directory.setSorting(QDir::Name);
	QStringList filters;
	for (auto kv : supported_extensions)
		filters << kv.first;
	directory.setNameFilters(filters);
	auto ret = directory.entryList();
	auto f = strcmpci<platform_case>;
	std::sort(ret.begin(), ret.end(), f);
	return ret;
}

bool check_and_clean_path(QString &path){
	QDir directory(path);
	if (!directory.exists()){
		return false;
	}
	path = directory.cleanPath(path);
	path = directory.toNativeSeparators(path);
	return true;
}
