#include "stdafx.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <sys/cdefs.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "FileTrackerLinux.h"

FileTrackerLinux FileTrackerLinux::create()
{
	FileTrackerLinux tracker;

	tracker.m_FD = inotify_init();
	if (tracker.m_FD < 0)
		throw std::runtime_error(strerror(errno));

	FD_ZERO(&tracker.m_descriptor_set);

	return tracker;
}

FileTrackerLinux::~FileTrackerLinux() = default;

void FileTrackerLinux::track_directory(const std::string& directory, EventHandlerFn handler)
{
	int wd = inotify_add_watch(m_FD, directory.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_MOVED_FROM | IN_DELETE);
	if (wd < 0)
		throw std::runtime_error(strerror(errno));

	m_trackers.emplace(wd, TrackInfo{ directory, handler });
}

void FileTrackerLinux::stop_track(const std::string& directory)
{
	auto it = std::find_if(std::begin(m_trackers), std::end(m_trackers), [&directory](auto tracker)
		{
			return tracker.second.first == directory;
		});

	if (it == std::end(m_trackers))
		return;

	inotify_rm_watch(m_FD, it->first);
	m_trackers.erase(it);
}

void FileTrackerLinux::check()
{
	FD_SET(m_FD, &m_descriptor_set);
	if (!FD_ISSET(m_FD, &m_descriptor_set))
		return; // TODO: meaningful error

	timeval timeout{0, 0};
	const auto res = select(m_FD + 1, &m_descriptor_set, NULL, NULL, &timeout);
	if (res < 0)
		throw std::runtime_error(strerror(errno));

	static constexpr auto s_buff_size = (sizeof(inotify_event) + FILENAME_MAX) * 1024;
	char buff[s_buff_size] = {0};
	const auto size = read(m_FD, buff, s_buff_size);

	ssize_t current_begin{0};
	while (current_begin < size)
	{
		auto* event = (inotify_event*)&buff[current_begin];
		handle_on_close(event);
		current_begin += sizeof(inotify_event) + event->len;
	}
}

FileTrackerLinux::FileTrackerLinux() = default;

void FileTrackerLinux::handle_on_close(inotify_event* event)
{
	assert(event);

	if (IN_CLOSE & event->mask)
	{
		auto& [dir, handler] = m_trackers[event->wd];
		std::string file_name(event->name, event->len);
		handler(dir, file_name);
	}
}
