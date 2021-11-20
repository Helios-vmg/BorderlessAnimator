/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef DIRECTORYLISTING_H
#define DIRECTORYLISTING_H

#include <QString>
#include <QStringList>
#include <QFuture>
#include <vector>
#include <QtCore/qatomic.h>
#include <memory>
#include <unordered_map>

void initialize_supported_extensions();

bool check_and_clean_path(QString &path);

class ExtensionIterator{
	std::unique_ptr<void, void (*)(void *)> pimpl;
	ExtensionIterator(const void *);
	static void release_pointer(void *);
public:
	ExtensionIterator(const ExtensionIterator &);
	const ExtensionIterator &operator=(const ExtensionIterator &other);
	static ExtensionIterator begin();
	static ExtensionIterator end();
	const ExtensionIterator &operator++();
	bool operator==(const ExtensionIterator &other) const;
	bool operator!=(const ExtensionIterator &other) const{
		return !(*this != other);
	}
	QString operator*() const;
};

class SupportedExtensions{
public:
	ExtensionIterator begin(){
		return ExtensionIterator::begin();
	}
	ExtensionIterator end(){
		return ExtensionIterator::end();
	}
};

#endif // DIRECTORYLISTING_H
