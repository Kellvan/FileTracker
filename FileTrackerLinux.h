#pragma once

#include "IFileTracker.h"

// Forward declaration.
struct inotify_event;

/// The class implements file tracking for Linux platform based on inotify.
/// Will call back each time a file been closed.
class FileTrackerLinux final : public IFileTracker
{
public:
	/// Creates a new file tracker.
	/// \exception std::runtime_error Throws if inotify failed to init.
	/// \return Initialized and ready-to-use tracker.
	static FileTrackerLinux create();

	/// A default destructor.
	~FileTrackerLinux();

	/// Adds the directory with the handler to tracking.
	/// \exception std::runtime_error Throws if system failed to add new a watch to inotify.
	/// \param directory The path to directory.
	/// \param handler The callback to call on file closing.
	void track_directory(const std::string& directory, EventHandlerFn handler);

	/// Removes the directory from tracking. Do nothing if the directory wasn't added previously.
	/// \param directory The path to directory that should be removed.
	void stop_track(const std::string& directory);

	/// Checks the directories on new closing events.
	/// \exception std::runtime_error Throws if failed to select file descriptor.
	void check();

private:
	/// A default constructor.
	FileTrackerLinux();

	/// Checks that the event is on close and calls related handler
	/// \param event The event to process. Must be not null.
	void handle_on_close(inotify_event* event);

	/// Represents the tracking info (directory and handler) from a user.
	using TrackInfo = std::pair<std::string, IFileTracker::EventHandlerFn>;

	/// System watch id to track into dictionary.
	std::map<std::uint32_t, TrackInfo> m_trackers;

	/// inotify file descriptor.
	int m_FD;

	/// File descriptor set.
	fd_set m_descriptor_set;
};
